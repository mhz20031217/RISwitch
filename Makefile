# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2023 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
define git_commit
	-@git add $(NEMU_HOME)/.. -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(echo "> $(1)" && uname -a && uptime) | git commit -F - $(GITFLAGS)
	-@sync
endef

_default:
	@echo "Please run 'make' under subprojects."

.PHONY: default 
