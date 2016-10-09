# i3ipc-doodle
A small test of of using the i3 IPC protocoll.

## FILES
Doodle comes with the following files on delivery
 - One global configuration file in $XDG_CONFIG_DIRS/doodle/config.cpp (either in /usr/share/* or /etc/*)
 - One example job file in $XDG_CONFIG_DIRS/doodle/example_job.job
 => run doodle_daemon --copy-config to copy the configuration files to $XDG_CONFIG_HOME/doodle/*

When running, doodle creates a time-file of the same name for every .job file in the configuration, so the time-file for example_job.job would be in $XDG_DATA_HOME/doodle/example_job.times. To save space, the time-files are not json files, but the file format is simple:
The first line contains the total time for that job in seconds, the remaining lines hold the details. Each line represents one time slice.
Each line contains two numbers of which the first is the beginning of a time slice and the second is the time spent on that job during the time slice.

## TODO
 - Implement an "action" list for jobs: When a job becomes active, the active workspace and window name is matched to a second list in the job file and if it matches, the corresponding action/system command is started.










Window events:
--------------
One easy way to group windows is to find a common string in the window name.
For some programs, the neccessary configuration options are shown to configure the window title.
 - Bash/ZSH with XTERM/URXVT:
	precmd () {echo -ne "\033]0;ZSH: $PWD\007"}
 - ranger (file manager):
	# Set a title for the window?
	set update_title true
	# Set the title to "ranger" in the tmux program?
	set update_tmux_title true
	# Shorten the title if it gets long?  The number defines how many
	# directories are displayed at once, 0 turns off this feature.
	set shorten_title 0
	
	# Abbreviate $HOME with ~ in the titlebar (first line) of ranger?
	##set tilde_in_titlebar false
	# mox
	set tilde_in_titlebar false
 - vimperator (addon for firefox)
	:set titlestring="AAAAAAAAA"


