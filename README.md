# i3ipc-doodle
A small test of of using the i3 IPC protocoll.


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


