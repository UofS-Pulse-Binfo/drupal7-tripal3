<?php
/**
 * @file
 * API Functions for Drupal to interact with Daemons.
 */

/**
 * Registers Daemons with the Daemon API.
 *
 * @return
 *    An array of daemons to be registered with keys being daemon machine names.
 */
function hook_daemon_api_info() {
  $daemon = array();

  // This is an example daemon which just sleeps for random amounts of time.
  $daemon['mymodule_daemon'] = array(
    // The machine name of the daemon (same as key above).
    'machine_name' => 'mymodule_daemon',
    // A human-readable name for your daemon.
    'name' => 'My Daemon',
    // This module (ie: the module implementing the daemon).
    'module' => 'mymodule',
    // The class extending DrushDaemon and implementing your daemon-specific
    // functionality. This class should be in a [classname].inc file in your
    // modules base directory.
    'class' => 'MyModuleDaemon',
    // OPTIONAL: Define this if your module doesn't follow the rule mentioned
    // in the above comment. The name and path to the file containing the
    // daemon class assuming your module folder as the root.
    'class_file' => 'MyModuleDaemon.inc',
  );

  return $daemon;
}

/**
 * Retrieve the registration info for a given daemon.
 *
 * @param $daemon_machine_name
 *   The machine name of the daemon to return information for.
 * @return
 *   A keyed array containing information about the daemon.
 */
function drushd_get_daemon_info($daemon_machine_name) {

  // Retrieve information on all daemons.
  $registered_daemons = module_invoke_all('daemon_api_info');
  if (!isset($registered_daemons[$daemon_machine_name])) {

    watchdog(
      'drushd',
      'Daemon :name not registered. Please implement hook_daemon_api_info() in your module.',
      array(':name' => $daemon_machine_name),
      WATCHDOG_WARING
    );
    return FALSE;
  }

  $daemon_info = $registered_daemons[$daemon_machine_name];

  // Set the class file default if needed.
  if (!isset($daemon_info['class_file'])) {
    $daemon_info['class_file'] = $daemon_info['class'] . '.inc';
  }

  // Set full path to class file.
  $daemon_info['class_filepath'] = DRUPAL_ROOT . '/'
    . drupal_get_path('module', $daemon_info['module'])
    . '/' . $daemon_info['class_file'];

  // Set the default files directory if needed.
  // This is where to look for the lock and status files.
  $daemon_info['files_dir'] = file_directory_temp();

  return $daemon_info;
}

/**
 * Load PHP Classes for the given daemon to function.
 *
 * @param $daemon_machine_name
 *   The machine name of the daemon to load the classes of.
 * @param $daemon_info (optional)
 *   Info for the current daemon from drushd_get_daemon_info();
 */
function drushd_load_daemon_classes($daemon_machine_name, $daemon_info = NULL) {

  // Retrieve information about the daemon if it wasn't given to us.
  if ($daemon_info === NULL) {
    $daemon_info = drushd_get_daemon_info($daemon_machine_name);
    if (!$daemon_info) return FALSE;
  }

  // Include the PHP-Daemon Library.
  libraries_load('PHP-Daemon');

  // Include the base DrupalDaemon class.
  require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'drushd') . '/daemon_classes/DrushDaemon.inc';

  // Include our daemon class.
  require_once $daemon_info['class_filepath'];

  return TRUE;
}

/**
 * Checks to see whether the specified daemon is running.
 *
 * @param $daemon_machine_name
 *   The machine name of the daemon you would like to check as specified in hook_daemon_api_info().
 * @return
 *   TRUE if the daemon is running and FALSE otherwise.
 */
function drushd_is_daemon_running($daemon_machine_name, $daemon_info = NULL) {

  // Retrieve information about the daemon if it wasn't given to us.
  if ($daemon_info === NULL) {
    $daemon_info = drushd_get_daemon_info($daemon_machine_name);
    if (!$daemon_info) return FALSE;
  }

  // Load the necessary classes.
  $success = drushd_load_daemon_classes($daemon_machine_name, $daemon_info);
  if (!$success) return FALSE;

  // Retrieve the path to the lock file.
  $lock_file = call_user_func(array($daemon_info['class'], 'getLockFile'), $daemon_info['class'], $daemon_info['files_dir']);

  // If it exists then the daemon is running.
  if (file_exists($lock_file)) {
    return $lock_file;
  }
  // Otherwise, it is not.
  else {
    return FALSE;
  }
}

/**
 * Gets the name of the file containing the status of the daemon.
 *
 * @param $daemon_machine_name
 *   The machine name of the daemon you would like to check as specified in hook_daemon_api_info().
 * @return
 *   The full path to the file if it is readable and FALSE otherwise.
 */
function drushd_get_daemon_status_file($daemon_machine_name, $daemon_info = NULL) {

  // Retrieve information about the daemon if it wasn't given to us.
  if ($daemon_info === NULL) {
    $daemon_info = drushd_get_daemon_info($daemon_machine_name);
    if (!$daemon_info) return FALSE;
  }

  // Load the necessary classes.
  $success = drushd_load_daemon_classes($daemon_machine_name, $daemon_info);
  if (!$success) return FALSE;

  $status_file = call_user_func(array($daemon_info['class'], 'getStatusFile'), $daemon_info['class'], $daemon_info['files_dir']);
  if (is_readable($status_file)) {
    return $status_file;
  }
  else {
    return FALSE;
  }
}

/**
 * Check to see if the daemon is running and if not (and that it was running before)
 * send the site administrator an email.
 *
 * @param $daemon_machine_name
 *   The machine name of the daemon you would like to check as specified in hook_daemon_api_info().
 */
function drushd_notify_admin_when_daemon_down($daemon_machine_name = 'drush_daemon') {
  global $base_url;
  $site_name = variable_get('site_name');
  $admin_user = user_load(1);

  // If the daemon is not running...
  if (!drushd_is_daemon_running($daemon_machine_name)) {

    // Determine the last known status for the daemon.
    $status_file = drushd_get_daemon_status_file($daemon_machine_name);
    $status = shell_exec("cat $status_file");
    if (!empty($status)) {
      $status_report = "The last known status for this daemon is:\n";
      $status = unserialize($status);
      foreach ($status as $name => $value) {
        // Check for boolean TRUE to print nicely.
        if ($value === TRUE) {
          $status_report .= check_plain($name) . ": TRUE\n";
        }
        // Check for boolean FALSE to print nicely.
        elseif ($value === FALSE) {
          $status_report .= check_plain($name) . ": FALSE\n";
        }
        // Otherwise just assume it's a string and print as is.
        else {
          $status_report .= check_plain($name) . ": " . check_plain($value) . "\n";
        }
      }
    }
    // We do not need to inform anyone of the death of someone who never lived.
    else {
      return '';
    }

    // Set who this email is being sent from (ie: from KnowPulse)
    $from = variable_get('site_mail', '');
    $to = $admin_user->mail;

    // Compile the body of the email to send.
    $body[] = 'Daemon Machine Name: ' . $daemon_machine_name;
    $body[] = 'Website: ' . $site_name . ' (' . $base_url . ')';
    $body[] = 'Dear '.$admin_user->name.",\n"
      . "We regret to inform you that your $daemon_machine_name daemon has perished. Although we are uncertain as to the cause of his/her death, we have attached their last communication of status. We recommend you use drush to restart your daemon at your earliest convenience."
      . "\nSincerely,\n~".$site_name;
    $body[] = $status_report;

    $params = array(
      'body' => $body,
      'subject' => $site_name . ': Drush Daemon (' . $daemon_machine_name . ') is no longer running...',
    );
    return drupal_mail('drushd', 'drushd', $to, language_default(), $params, $from, TRUE);
  }

  return FALSE;
}

