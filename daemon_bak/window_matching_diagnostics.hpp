#pragma once
// These functions are part of class Window_matching. This file may only be included _in_the_body_of_class_Window_matching!





bool operator==(const Window_matching& other) const
{
	if(matchers.win_names.include != other.matchers.win_names.include) return false;
	if(matchers.win_names.exclude != other.matchers.win_names.exclude) return false;
	if(matchers.ws_names.include  != other.matchers.ws_names.include)  return false;
	if(matchers.ws_names.exclude  != other.matchers.ws_names.exclude)  return false;
	return true;
}

matchers_t& get_matchers(void)
{
	return matchers;
}
