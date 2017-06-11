
This workspace was setup for Drupal 7 development. I have included all the software I regularily use during development; however, keep in mind this stack may not suit your development workflow. This Drupal7 workspace was setup with PostgreSQL rather then MySQL to aid in development of Tripal.

## Stack
This workspace has the following installed:
- Ubuntu 14.04.5 LTS (trusty)
- Apache 2.4.7
- PHP 5.5.9
- PostgreSQL 9.3.15
- Drupal 7.55
- Drush 8.1.12

## Quickstart
### Initial Cloud9 Workspace setup.
1. On your dashboard click "Create a new workspace."
2. Enter workspace information
  - Provide a name and description for your workspace.
  - Under "Clone from Git or Mercurial URL (optional)" enter "https://github.com/UofS-Pulse-Binfo/drupal7.git"
  - Under "Choose a template" pick "Apache & PHP5"
  - Click "Create Workspace."
3. Install Drush
```
composer global require drush/drush:8.*
sed -i '1i export PATH="$HOME/.composer/vendor/bin:$PATH"' $HOME/.bashrc
source $HOME/.bashrc
```
4. Click "Run Project" above to start apache2
5. Update sites/default/default.settings.php as follows:
> Scroll down to the line: `# base_url = 'http://www.example.com';`, remove the crosshatch (#), and replace the URL with the following: `https://projectname-username.c9.io4` where you have replaced username with your own username and projectname with the name of your workspace.
6. Install your Drupal site: `./create_fresh.sh`
7. In a new tab, open the Site URL and sign in using the credentials listed.

### Create a Fresh Drupal install.
1. Click "Run Project" above to start apache2
2. Install your Drupal site: `./create_fresh.sh`
3. In a new tab, open the Site URL and sign in using the credentials listed.

### Shutdown for the day.
The follow commands will shut down your site for the day. **You're local files and database will be saved.**
1. Make sure all your changes have been committed (if appropriate) and pushed.
2. Stop PostgreSQL: `sudo service postgresql stop`
3. Stop Apache: Click "Stop" above.

### Restart existing site.
1. Start Apache: Click "Run Project" above.
2. Start PostgreSQL: `sudo service postgresql start`
3. Open your site as specified in the "Apache" tab.

### Useful locations
- Apache Logs: ~/lib/apache2/log

## Troubleshooting

### Drush is not installed.
Setting up Drush is easy; simply execute the following commands:
```
composer global require drush/drush:8.*
sed -i '1i export PATH="$HOME/.composer/vendor/bin:$PATH"' $HOME/.bashrc
source $HOME/.bashrc
```

### Your site is not displaying properly (white background with minimal styling).
This is due to Drupal trying to load certain stylesheets over HTTP and the page is loaded with HTTPS. To fix this by enabling SSL, go to the Terminal, and execute the following:

```
cd $HOME/workspace/sites/default
sudo vim settings.php
```

Scroll down to the line: `# base_url = 'http://www.example.com';`, remove the crosshatch (#), and replace the URL with the following: `https://projectname-username.c9.io4` where you have replaced username with your own username and projectname with the name of your workspace. Save the file and refresh your drupal site in the browser.
