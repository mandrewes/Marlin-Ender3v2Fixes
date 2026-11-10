SCRIPTS_DIR := buildroot/share/scripts
MAKESCRIPTS_DIR := buildroot/share/make
CONTAINER_RT_BIN := docker
CONTAINER_RT_OPTS := --rm -v $(PWD):/code -v platformio-cache:/root/.platformio
CONTAINER_IMAGE := marlin-dev
UNIT_TEST_CONFIG ?= default
PRETTIER_VERSION ?= 3.9.6

# Find a Python 3 interpreter
ifeq ($(OS),Windows_NT)
	# Windows: use `where` – fall back through the three common names
	PYTHON := $(shell which python 2>nul || which python3 2>nul || which py 2>nul)
	# Windows: Use Python script to find pins files
	ALL_PINS := $(shell $(PYTHON) $(MAKESCRIPTS_DIR)/find.py Marlin/src/pins -mindepth 2 -name 'pins_*.h')
else
	# POSIX: use `command -v` – prefer python3 over python
	PYTHON := $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null)
	# Unix/Linux: Use find command
	ALL_PINS := $(shell find Marlin/src/pins -mindepth 2 -name 'pins_*.h')
endif

PINSPATH ?=
PINSPATH := $(patsubst Marlin/src/pins/%,%,$(PINSPATH))
PINSPATH := $(patsubst %/,%,$(PINSPATH))

# If PINSPATH already contains %, use it directly.
# Otherwise append /% to match all files below the path.
PINS := $(if $(PINSPATH),$(filter Marlin/src/pins/$(PINSPATH)/%,$(ALL_PINS)),$(ALL_PINS))

# Check that the found interpreter is Python 3
# Error if there's no Python 3 available
ifneq ($(strip $(PYTHON)),)
	PYTHON_VERSION := $(shell $(PYTHON) -c "import sys; print(sys.version_info[0])" 2>/dev/null)
	ifneq ($(PYTHON_VERSION),3)
		$(error $(PYTHON) is not Python 3 – install a Python‑3.x interpreter or adjust your PATH)
	endif
else
	$(error No Python executable found – install Python 3.x and make sure it is in your PATH)
endif

help:
	@echo "Tasks for local development:"
	@echo "make marlin                    : Build Marlin for the configured board"
	@echo "make flash-ender               : Bump version, clean build, write firmware.bin to SD card"
	@echo "make flash-ender-display       : Write DWIN_SET display firmware to SD card"
	@echo "make flash-ender-all           : Both mainboard and display firmware on card"
	@echo "make format-pins -j            : Reformat all pins files (-j for parallel execution)"
	@echo "make format-pins -j PINSPATH=dir : Reformat only pins files under dir"
	@echo "make validate-lines -j         : Validate line endings, fails on trailing whitespace, etc."
	@echo "make validate-pins -j          : Validate all pins files, fails if any require reformatting"
	@echo "make validate-boards -j        : Validate boards.h and pins.h for standards compliance"
	@echo "make validate-urls             : Validate URLs in source files"
	@echo "make tests-single-ci           : Run a single test from inside the CI"

# ─── Ender 3 V2 SD-card flash targets ────────────────────────────────────────
#
# Usage:
#   make flash-ender          – bump version, clean build, copy firmware to card
#   make flash-ender-display  – copy DWIN_SET display assets to card
#   make flash-ender-all      – both of the above
#
# The SD card must be inserted and visible as /Volumes/ENDER before running.
# Each flash uses a unique filename (MAxx.bin) so the Creality bootloader
# never skips it due to a cached filename check.
# macOS resource-fork (._*) files are stripped before ejecting.
#
ENDER_SD        := /Volumes/ENDER
VERSION_FILE    := Marlin/Version.h
DWIN_SRC        := LCD\ Files/DWIN_SET
BUILD_DIR_E     := .pio/build/STM32F103RE_creality
ENV             := STM32F103RE_creality

# Extract current patch number from SHORT_BUILD_VERSION "2.1.X-MA" and increment it
_CUR_VER        := $(shell sed -n 's/.*SHORT_BUILD_VERSION "\([^"]*\)".*/\1/p' $(VERSION_FILE))
_VER_BASE       := $(shell echo "$(_CUR_VER)" | sed 's/-MA//')
_MAJOR          := $(shell echo "$(_VER_BASE)" | cut -d. -f1)
_MINOR          := $(shell echo "$(_VER_BASE)" | cut -d. -f2)
_PATCH          := $(shell echo "$(_VER_BASE)" | cut -d. -f3)
_NEW_PATCH      := $(shell echo "$(shell expr $(_PATCH) + 1)")
NEW_VERSION     := $(_MAJOR).$(_MINOR).$(_NEW_PATCH)-MA
# Short filename: MA + major + minor + newpatch, e.g. MA2111.bin
BIN_NAME        := MA$(_MAJOR)$(_MINOR)$(_NEW_PATCH).bin

.PHONY: flash-ender flash-ender-display flash-ender-all _check-card _bump-version _clean-build _copy-firmware _copy-display _eject

_check-card:
	@test -d "$(ENDER_SD)" || { echo "ERROR: SD card not found at $(ENDER_SD). Insert the card first."; exit 1; }

_bump-version:
	@echo "Bumping version: $(_CUR_VER) → $(NEW_VERSION)"
	@sed -i '' 's/SHORT_BUILD_VERSION "[^"]*"/SHORT_BUILD_VERSION "$(NEW_VERSION)"/' $(VERSION_FILE)

_clean-build:
	@echo "Cleaning and rebuilding firmware..."
	pio run -e $(ENV) --target clean
	pio run -e $(ENV)
	@test -n "$$(ls -t $(BUILD_DIR_E)/firmware-*.bin 2>/dev/null | head -1)" || { echo "ERROR: No firmware binary found after build."; exit 1; }
	@echo "Built: $$(ls -t $(BUILD_DIR_E)/firmware-*.bin | head -1)"
	@strings "$$(ls -t $(BUILD_DIR_E)/firmware-*.bin | head -1)" | grep -q "$(NEW_VERSION)" || { echo "ERROR: Version $(NEW_VERSION) not found in binary!"; exit 1; }
	@echo "Version $(NEW_VERSION) confirmed in binary."

_copy-firmware: _check-card
	@echo "Clearing card..."
	@rm -f $(ENDER_SD)/*.bin $(ENDER_SD)/*.BIN $(ENDER_SD)/*.CUR $(ENDER_SD)/*.cur $(ENDER_SD)/._*
	@rm -rf $(ENDER_SD)/DWIN_SET
	@echo "Copying $(BIN_NAME) to card..."
	@cp "$$(ls -t $(BUILD_DIR_E)/firmware-*.bin | head -1)" "$(ENDER_SD)/$(BIN_NAME)"
	@# Strip macOS resource forks
	@dot_clean "$(ENDER_SD)" 2>/dev/null || true
	@rm -f "$(ENDER_SD)/._$(BIN_NAME)"
	@echo "Card contents:"
	@ls -la $(ENDER_SD)/

_copy-display: _check-card
	@echo "Clearing card..."
	@rm -f $(ENDER_SD)/*.bin $(ENDER_SD)/*.BIN $(ENDER_SD)/*.CUR $(ENDER_SD)/*.cur $(ENDER_SD)/._*
	@rm -rf $(ENDER_SD)/DWIN_SET
	@echo "Copying DWIN_SET..."
	@cp -r $(DWIN_SRC) $(ENDER_SD)/DWIN_SET
	@dot_clean $(ENDER_SD) 2>/dev/null || true
	@rm -f $(ENDER_SD)/._DWIN_SET
	@find $(ENDER_SD)/DWIN_SET -name "._*" -delete
	@echo "Card contents:"
	@ls -la $(ENDER_SD)/
	@ls $(ENDER_SD)/DWIN_SET/

_eject:
	@echo "Ejecting card..."
	@diskutil eject $(ENDER_SD) && echo "Card ejected safely. Insert into Ender (power off first), then power on."

flash-ender: _check-card _bump-version _clean-build _copy-firmware _eject

flash-ender-display: _copy-display _eject

flash-ender-all: _check-card _bump-version _clean-build _copy-firmware
	@cp -r $(DWIN_SRC) $(ENDER_SD)/DWIN_SET
	@dot_clean $(ENDER_SD) 2>/dev/null || true
	@rm -f $(ENDER_SD)/._DWIN_SET
	@find $(ENDER_SD)/DWIN_SET -name "._*" -delete
	@echo "Card contents (firmware + display):"
	@ls -la $(ENDER_SD)/
	@ls $(ENDER_SD)/DWIN_SET/
	$(MAKE) _eject

marlin:
	./buildroot/bin/mftest -a
.PHONY: marlin

clean:
	rm -rf .pio/build*

tests-single-ci:
	export GIT_RESET_HARD=true
	$(MAKE) tests-single-local TEST_TARGET=$(TEST_TARGET) PLATFORMIO_BUILD_FLAGS=-DGITHUB_ACTION

tests-single-local:
	@if ! test -n "$(TEST_TARGET)" ; then echo "***ERROR*** Set TEST_TARGET=<your-module> or use make tests-all-local" ; return 1; fi
	export PATH="./buildroot/bin/:./buildroot/tests/:${PATH}" \
	  && export VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) \
	  && run_tests . $(TEST_TARGET) "$(ONLY_TEST)"

tests-single-local-docker:
	@if ! test -n "$(TEST_TARGET)" ; then echo "***ERROR*** Set TEST_TARGET=<your-module> or use make tests-all-local-docker" ; return 1; fi
	@if ! $(CONTAINER_RT_BIN) images -q $(CONTAINER_IMAGE) > /dev/null ; then $(MAKE) setup-local-docker ; fi
	$(CONTAINER_RT_BIN) run $(CONTAINER_RT_OPTS) $(CONTAINER_IMAGE) make tests-single-local TEST_TARGET=$(TEST_TARGET) VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) GIT_RESET_HARD=$(GIT_RESET_HARD) ONLY_TEST="$(ONLY_TEST)"

tests-all-local:
	@$(PYTHON) -c "import yaml" 2>/dev/null || (echo 'pyyaml module is not installed. Install it with "$(PYTHON) -m pip install pyyaml"' && exit 1)
	export PATH="./buildroot/bin/:./buildroot/tests/:${PATH}" \
	  && export VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) \
	  && for TEST_TARGET in $$($(PYTHON) $(MAKESCRIPTS_DIR)/get_test_targets.py) ; do \
	    if [ "$$TEST_TARGET" = "linux_native" ] && [ "$$(uname)" = "Darwin" ]; then \
	      echo "Skipping tests for $$TEST_TARGET on macOS" ; \
	      continue ; \
	    fi ; \
	    echo "Running tests for $$TEST_TARGET" ; \
	    run_tests . $$TEST_TARGET || exit 1 ; \
	    sleep 5; \
	  done

tests-all-local-docker:
	@if ! $(CONTAINER_RT_BIN) images -q $(CONTAINER_IMAGE) > /dev/null ; then $(MAKE) setup-local-docker ; fi
	$(CONTAINER_RT_BIN) run $(CONTAINER_RT_OPTS) $(CONTAINER_IMAGE) make tests-all-local VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) GIT_RESET_HARD=$(GIT_RESET_HARD)

unit-test-single-local:
	platformio run -t marlin_$(UNIT_TEST_CONFIG) -e linux_native_test

unit-test-single-local-docker:
	@if ! $(CONTAINER_RT_BIN) images -q $(CONTAINER_IMAGE) > /dev/null ; then $(MAKE) setup-local-docker ; fi
	$(CONTAINER_RT_BIN) run $(CONTAINER_RT_OPTS)  $(CONTAINER_IMAGE) make unit-test-single-local UNIT_TEST_CONFIG=$(UNIT_TEST_CONFIG)

unit-test-all-local:
	platformio run -t test-marlin -e linux_native_test

unit-test-all-local-docker:
	@if ! $(CONTAINER_RT_BIN) images -q $(CONTAINER_IMAGE) > /dev/null ; then $(MAKE) setup-local-docker ; fi
	$(CONTAINER_RT_BIN) run $(CONTAINER_RT_OPTS)  $(CONTAINER_IMAGE) make unit-test-all-local

USERNAME := $(shell whoami)
USER_ID  := $(shell id -u)
GROUP_ID := $(shell id -g)

.PHONY: setup-local-docker setup-local-docker-old

setup-local-docker:
	@echo "Building marlin-dev Docker image..."
	$(CONTAINER_RT_BIN) build -t $(CONTAINER_IMAGE) \
	  --build-arg USERNAME=$(USERNAME) \
	  --build-arg USER_ID=$(USER_ID) \
	  --build-arg GROUP_ID=$(GROUP_ID) \
	  -f docker/Dockerfile .
	@echo
	@echo "To run all tests in Docker:"
	@echo "  make tests-all-local-docker"
	@echo "To run a single test in Docker:"
	@echo "  make tests-single-local-docker TEST_TARGET=mega2560"

setup-local-docker-old:
	$(CONTAINER_RT_BIN) buildx build -t $(CONTAINER_IMAGE) -f docker/Dockerfile .

.PHONY: $(PINS) format-pins validate-pins

$(PINS): %:
	@echo "Formatting pins $@"
	@$(PYTHON) $(SCRIPTS_DIR)/pinsformat.py $< $@

format-pins: $(PINS)
	@echo "Processed $(words $(PINS)) pins files"

validate-pins: format-pins
	@echo "Validating pins files"
	@git diff --exit-code || (git status && echo "\nError: Pins files are not formatted correctly. Run \"make format-pins\" to fix.\n" && exit 1)

.PHONY: format-lines validate-lines validate-urls

format-lines:
	@echo "Formatting all sources"
	@$(PYTHON) $(SCRIPTS_DIR)/linesformat.py buildroot
	@$(PYTHON) $(SCRIPTS_DIR)/linesformat.py Marlin

validate-lines:
	@echo "Validating text formatting"
	@npx --yes prettier@$(PRETTIER_VERSION) --check . --editorconfig --object-wrap preserve --prose-wrap never

validate-urls:
	@echo "Checking URLs in source files"
	@$(MAKESCRIPTS_DIR)/check-urls.sh

BOARDS_FILE := Marlin/src/core/boards.h

.PHONY: validate-boards

validate-boards:
	@echo "Validating boards.h file"
	@$(PYTHON) $(MAKESCRIPTS_DIR)/validate_boards.py $(BOARDS_FILE) || (echo "\nError: boards.h file is not valid. Please check and correct it.\n" && exit 1)
