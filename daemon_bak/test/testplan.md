1. Create a dummy i3 ipc host which can send sequences of messages
2. Create test sequences for all fixed bugs
3. Use some framework to run the sequences
4. Keep sequences up to date



Tests:
- Create program i3_dummy, that iterates over lines in a sequence file (using libev to catch messages while waiting), returning on error
- Line format: send|receive, wait[min|max], message string // for sending, the min delay is used
- Errors:
	+ wrong message received
	+ message received out of the time interval
	+ no message received when one is expected





Test cases for Window_matching
 - Construct a Window_matching object with all fields set
    | All fields are set
 - Match windows
  + Query with matching window name and workspace
    | The object responds "true"
  + Query with non-matching window name or workspace
    | The object responds "false"
  + Query with excluded window name or workspace
    | The object responds "false"
 - Construct a Window_matching object with a blank field
  + Repeat the tests above


Test cases for Job:
 - Construct a Job object with all file functions replaced with dummies
  + With no time file
    | Times files is created
  + With all files
    | Jobname and path set accordingly
    | Timefile opened
    | Time copied from time file
    | Window matchers set accordingly
    | Actions and action matchers set accordingly
 - Start a job
    | all specified actions are called
    | the job is marked as active
 - Stop a job







Test cases for Doodle:
 - Construct and delete a Doodle object
 - Change active window
  + Transitions:
   * no job -> no job
   * no job -> job1
   * job1   -> job2
   * job2   -> job2
   * job2   -> no job
  + special cases:
   * no workspaces defined for a job
   * no window names define for a job
 - Change active workspace
  + Transistions are covered by window change since the window change function is called by on_workspace_change internally anyway.
 - Let user go idle -> go active again
 - Let user go idle -> change window while user is idle
 - Open a user communication socket
    | Master socket is opened
  + Test the connection
   * Open a first client connection
    | Client can send commands and receive responses
   * Open a second client connection
    | Client can send commands and receive responses
   * Close first client
    | second client connection still valid
   * Open a third client connection
    | first and third client can send and receive
    | send kill -> Both connections are closed
  + Test each command
   * Create a doodle with known properties
   * Open a client connection
    | Each command received the expected response
