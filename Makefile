.PHONY: clean all

all:
	@echo "----------Building project:[ bounds ]----------"
	@cd "bounds" && "$(MAKE)" -f  "makefile" && cd ..
	@echo "----------Building project:[ heuristic ]----------"
	@cd "heuristic" && "$(MAKE)" -f  "makefile" && cd ..
	@echo "----------Building project:[ multicore ]----------"
	@cd "multicore" && "$(MAKE)" -f  "makefile" && cd ..
	@echo "----------Building project:[ multicoreLL ]----------"
	@cd "multicoreLL" && "$(MAKE)" -f  "makefile" && cd ..
	@echo "----------Building project:[ distributed ]------"
	@cd "distributed" && "$(MAKE)" -f  "makefile" && cd ..

multicore:
	@echo "----------Building project:[ multicore ]----------"
	@cd "multicore" && "$(MAKE)" -f  "makefile" && cd ..

clean:
	@echo "----------Cleaning project:[ bounds ]----------"
	@cd "bounds" && "$(MAKE)" -f  "makefile" clean && cd ..
	@echo "----------Cleaning project:[ heuristic ]----------"
	@cd "heuristic" && "$(MAKE)" -f  "makefile" clean && cd ..
	@echo "----------Cleaning project:[ multicore ]----------"
	@cd "multicore" && "$(MAKE)" -f  "makefile" clean && cd ..
	@echo "----------Cleaning project:[ multicoreLL ]----------"
	@cd "multicoreLL" && "$(MAKE)" -f  "makefile" clean && cd ..
	@echo "----------Cleaning project:[ distributed ]------"
	@cd "distributed" && "$(MAKE)" -f  "makefile" clean && cd ..
