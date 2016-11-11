# i3ipc-doodle
Work time accounting for i3.
Monitors which window is focused and counts up the time clock for the job the window belongs to.

## Idioms
 - Job: A certain thing you want to do and keep track of how much time you spend doing it. For example, you want to know, just how long you worked on that assignment. A job has a number of windows pertaining to it and while one of those is focused, a check clock for that job counts up. See 

## Names
 - i3_socket: The socket used to communicate with i3. Created by i3.
 - comm_socket: The socket used to 









## FILES
Doodle comes with the following files on delivery
 - One global configuration file in $XDG_CONFIG_DIRS/doodle/config.cpp (either in /usr/share/* or /etc/*)
 - One example job file in $XDG_CONFIG_DIRS/doodle/example_job.job
 => run doodle_daemon --copy-config to copy the configuration files to $XDG_CONFIG_HOME/doodle/*

When running, doodle creates a time-file of the same name for every .job file in the configuration, so the time-file for example_job.job would be in $XDG_DATA_HOME/doodle/example_job.times. To save space, the time-files are not json files, but the file format is simple:
The first line contains the total time for that job in seconds, the remaining lines hold the details. Each line represents one time slice.
Each line contains two numbers of which the first is the beginning of a time slice and the second is the time spent on that job during the time slice.


## Defining a job
A job is defined in a job file in $DOODLE_CONFIG_DIR/jobs/. The file MYJOB.job defines the job MYJOB and has to be valid [JSON](http://www.json.org/). The following fields are supported for a job:
 - granularity: The accounting period, that is the maximum amount of seconds before a new timestamp is placed in the times file. Example: If set to 86400 (one day) you would end up with an entry for each day, telling you how long you worked on that job this day.
 - window_names: A list of strings used to match windows to the job. Each string is treated as a regular expression and matched against the name of the focussed window. If at least one string matches, the job is considered the active one. However, strings can be prepended with a bang (!) to make it a must-not-match entry. If such a string matches, the job can not be active.
 - workspace_names: Much like the window names. The current workspace is matched against the list of strings and at least one has to match. Similarily, no string prepended with a bang may match for the job to be active.
If the granularity is omitted, a default of 3600 (one hour) is assumed. If either the window- or workspace names are left out, they are not considered. So, to have a job that traces how much time you spend on workspace "foobar" leave out the window_names and set workspace_names to "foobar".







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


