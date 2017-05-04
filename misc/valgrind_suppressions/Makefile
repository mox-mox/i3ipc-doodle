# Generate a valgrind suppressor file for various libraries
#
# Usage:
# 1. Run make, it will generate a suppression file for valgrind.
# 2. Test your program with the suppression file: valgrind --suppressions=valgrind_suppressors ./my_program
#
# Adding new libraries
# 1. Add a small *ERRORLESS* (on the user side) working example program using the functionality you intend to use
# 2. Add the generated program to the variable ${executables}
# 3. Re-run make



executables := libnotify_minimal
#executables := libnotify_minimal buggy

valgrind_suppressors: ${executables}
	@rm -f $@
	@$(foreach exe,${executables}, \
		echo "Generating suppressions for ${exe}"; \
		valgrind --leak-check=full --show-reachable=yes --error-limit=no --gen-suppressions=all  ./${exe}  3>&1 1>&2 2>&3 | ./grindmerge >> $@;)


####################################################################################################################################################

# Buggy is just a test program. It does contain a memory leak and therefore allows testing suppressions
buggy: buggy.cpp
	g++ $< -o $@ -ggdb -O0

check-buggy: buggy
	@echo -e "\n\n========== Running valgrind for $< ==========\n\n"
	valgrind --leak-check=full --show-reachable=yes --error-limit=no ./$<

re-check-buggy: buggy valgrind_suppressors
	@echo -e "\n\n========== Running valgrind for $< WITH SUPPRESSIONS ==========\n\n"
	valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_suppressors ./$<



# A minimal test of libnotify. Should be ok on the user side, so all valgrind warning it raises can be suppressed (except maybe, if you are the author of libnotify)
libnotify_minimal: libnotify_minimal.cpp
	g++ $< -o $@ `pkg-config --cflags --libs libnotify` -ggdb -O0

check-libnotify_minimal: libnotify_minimal
	@echo -e "\n\n========== Running valgrind for $< ==========\n\n"
	valgrind --leak-check=full --show-reachable=yes --error-limit=no ./$<

re-check-libnotify_minimal: libnotify_minimal valgrind_suppressors
	@echo -e "\n\n========== Running valgrind for $< WITH SUPPRESSIONS ==========\n\n"
	valgrind --leak-check=full --show-reachable=yes --error-limit=no --suppressions=valgrind_suppressors ./$<









.PHONY: clean
clean:
	rm -f valgrind_suppressors ${executables}
