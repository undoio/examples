# This Makefile builds the example programs in this directory.

COMMON = -O0 -g -fno-stack-protector -D_XOPEN_SOURCE=600 -Wno-stringop-overflow
CFLAGS += $(COMMON) -std=c99
CXXFLAGS += $(COMMON) -std=c++11

# Set V to 1 (for instance, `make V=1`) to see the commands being invoked.
V = 0
ifeq ("$V","0")
	verbose := @
else
	verbose :=
endif

.PHONY: all
all: aio cache cache-cpp cache-distributed/cache-distributed cpubound deadlock hashtable hello-world linked-list malloc-var race simple sine stacksmash threads workers

aio: aio.c .libaio_h-stamp
	@printf "CC\taio\n"
	$(verbose)if [ ! -e ".libaio_h-stamp" ]; then \
		printf "ERROR\taio: libaio required\nInstall 'libaio-dev' on deb-based distributions (Ubuntu, Debian, etc.), or\n'libaio-devel' on rpm-based ones (Fedora, Red Hat, CentOS, etc.) to compile\nthis example.\n"; \
	else \
		$(CC) $(CFLAGS) $< -laio $(LDFLAGS) -o $@; \
	fi

cache: cache.c 
	@printf "CC\tcache\n"
	$(verbose)$(CC) $(CFLAGS) $< -lm $(LDFLAGS) -o $@

cache-cpp: cache-cpp.cpp .cxx-version-check
	@printf "CXX\tcache-cpp\n"
	$(verbose)if [ ! -e ".cxx-version-check" ]; then \
		printf "ERROR\tcache-cpp: C++ 11 support required\nC++11 (GCC >= $(CXX_VERSION_MIN)) is required to build this example.\n"; \
	else \
		$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@; \
	fi

cache-distributed/cache-distributed: cache-distributed/cache-distributed.c 
	@printf "CC\tcache-distributed/cache-distributed\n"
	$(verbose)$(CC) $(CFLAGS) $< -lm $(LDFLAGS) -o $@

cpubound: cpubound.cpp .cxx-version-check
	@printf "CXX\tcpubound\n"
	$(verbose)if [ ! -e ".cxx-version-check" ]; then \
		printf "ERROR\tcpubound: C++ 11 support required\nC++11 (GCC >= $(CXX_VERSION_MIN)) is required to build this example.\n"; \
	else \
		$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@; \
	fi

deadlock: deadlock.c 
	@printf "CC\tdeadlock\n"
	$(verbose)$(CC) $(CFLAGS) $< -lpthread $(LDFLAGS) -o $@

hashtable: hashtable.c 
	@printf "CC\thashtable\n"
	$(verbose)$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

hello-world: hello-world.c 
	@printf "CC\thello-world\n"
	$(verbose)$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

linked-list: linked-list.c 
	@printf "CC\tlinked-list\n"
	$(verbose)$(CC) $(CFLAGS) $< -lpthread $(LDFLAGS) -o $@

malloc-var: malloc-var.c 
	@printf "CC\tmalloc-var\n"
	$(verbose)$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

race: race.cpp .cxx-version-check
	@printf "CXX\trace\n"
	$(verbose)if [ ! -e ".cxx-version-check" ]; then \
		printf "ERROR\trace: C++ 11 support required\nC++11 (GCC >= $(CXX_VERSION_MIN)) is required to build this example.\n"; \
	else \
		$(CXX) $(CXXFLAGS) $< -lpthread $(LDFLAGS) -o $@; \
	fi

simple: simple.c 
	@printf "CC\tsimple\n"
	$(verbose)$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

sine: sine.c 
	@printf "CC\tsine\n"
	$(verbose)$(CC) $(CFLAGS) $< -lm $(LDFLAGS) -o $@

stacksmash: stacksmash.c 
	@printf "CC\tstacksmash\n"
	$(verbose)$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

threads: threads.c 
	@printf "CC\tthreads\n"
	$(verbose)$(CC) $(CFLAGS) $< -lpthread $(LDFLAGS) -o $@

workers: workers.c 
	@printf "CC\tworkers\n"
	$(verbose)$(CC) $(CFLAGS) $< -lpthread $(LDFLAGS) -o $@

.PHONY: clean
clean:
	$(verbose)rm -f .libaio_h-stamp .cxx-version-check
	$(verbose)rm -f aio cache cache-cpp cache-distributed/cache-distributed cpubound deadlock hashtable hello-world linked-list malloc-var race simple sine stacksmash threads workers

.PHONY: help
help:
	@echo "This Makefile can be used to build the example programs in this directory:"
	@echo "    $$ make [aio|cache|cache-cpp|cache-distributed/cache-distributed|cpubound|deadlock|hashtable|hello-world|linked-list|malloc-var|race|simple|sine|stacksmash|threads|workers]"

CXX_VERSION_MIN="4.8.1"
CXX_VERSION=$(shell gcc --version | grep "gcc" | tr " " "\n" | grep -P "^\d+\.\d+\.\d+$$")
VERSION_OK=$(shell printf "%s\n" "$(CXX_VERSION)" $(CXX_VERSION_MIN) | sort -VC; echo $$?)
.cxx-version-check:
	@printf "CHECK\tC++ 11 support\n"
ifeq ("$(CXX)","g++")
ifneq ("$(VERSION_OK)","0")
	$(verbose)touch "$@"
endif
else
	$(verbose)touch "$@"
endif

.libaio_h-stamp:
	@printf "CHECK\tlibaio\n"
	$(verbose)if echo "#include <libaio.h>" | $(CC) -E - >/dev/null 2>&1; then \
		touch "$@"; \
	fi

