#!/bin/bash
echo ""
echo "Creates a clean drupal database and installs a Drupal 7 site."
echo "Your previous database will be lost!"

read -p "Would you like to continue? (y/n): " yn
if [ "$yn" != "y" ]; then
  exit 1
fi
echo ""

PASSWORD=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 16 | head -n 1)

# Start PostgreSQL server.
echo "1. Start PostgreSQL server."
sudo service postgresql start
echo ""

# Drop the current user/database.
echo "2. Remove existing installation."
psql --command="DROP DATABASE IF EXISTS drupal7"
psql --command="DROP USER IF EXISTS web_admin"
echo ""

# Re-create the user/database.
echo "3. Re-create Database."
psql --command="CREATE USER web_admin WITH PASSWORD '$PASSWORD'"
psql --command="CREATE DATABASE drupal7 WITH OWNER web_admin ENCODING 'UTF8' TEMPLATE template0"
echo ""

# Setup a new settings.php file
sudo cp sites/default/default.settings.php sites/default/settings.php
sudo chmod a+w sites/default/settings.php

# Next, install Drupal.
echo "4. Install Drupal via Drush"
drush site-install standard \
  --db-url="pgsql://web_admin:$PASSWORD@localhost/drupal7" \
  --site-name="Drupal 7 Dev" \
  --account-name=drupaladmin \
  --account-pass="$PASSWORD"

# Set the site slogan to show build date.
drush php-eval "variable_set('site_slogan','Built on '.date('YMd h:i'));"

# Re-protect the settings file.
sudo chmod a-w sites/default/settings.php
echo "Write permissions removed from sites/default/settings.php."
echo ""

# Enable common module.
echo "5. Enabling common modules and Tripal dependencies."
drush pm-disable overlay
drush pm-enable ctools views views_ui devel redirect entity link date ds field_group field_group_table libraries
drush pm-enable drushd
echo ""

# Get Tripal
echo "6. Clone Tripal 3.x"
cd sites/all/modules
[ -d tripal ] && sudo rm -r tripal
git clone https://github.com/tripal/tripal.git --branch 7.x-3.x --single-branch
echo ""

# Enable Tripal
echo "7. Enable common Tripal modules."
drush pm-enable tripal tripal_chado tripal_ds tripal_ws tripal_daemon
echo ""

# User Feedback.
echo "Assuming there are no errors shown above, you have successfully re-installed Drupal."
echo "Site URL: (See Apache Tab)"
echo "Drupal Username: drupaladmin"
echo "Drupal Password: $PASSWORD"