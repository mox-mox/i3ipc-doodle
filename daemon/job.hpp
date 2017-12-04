#pragma once
#include "main.hpp"

// File jobname.job:
//
// [window-names]
// include="foo*", "bar", "baz" # Regular expressions that are used to match the window title
// exclude="foo", "bar", "baz"
//
// [workspace-names]
// include="foo", "bar", "baz"
// exclude="foo", "bar", "baz"
//
// [timefile]
// filename=/path/to/file
// granularity=1h
//
// [actions]
// # TODO
//









class Job
{
	std::shared_ptr<uvw::Loop> loop;
	std::string jobname;


	std::vector<std::string> window_include;
	std::vector<std::string> window_exclude;

	std::vector<std::string> workspace_include;
	std::vector<std::string> workspace_exclude;

	public:
	Job(const fs::path& jobfile_path, std::shared_ptr<uvw::Loop> loop);




};

