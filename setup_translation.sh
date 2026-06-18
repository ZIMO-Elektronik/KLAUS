#!/bin/bash
set -e

show_help() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Options:"
  echo "  -e, --extract   Extract strings from .slint, .cpp and .hpp files and initialise de and en translations"
  echo "  -b, --build     Build / Compile the .po files into binary .mo runtime files"
  echo "  -h, --help      Show this helptext"
  echo ""
  echo "Example: $0 -e -b  ## Do everything at once"
  exit 0
}

## Parse options wiht getopt
OPTIONS=$(getopt -o ebh --long extract,build,help -n "$0" -- "$@")\

## Abort if any invalid option exists
if [ $? -ne 0 ]; then
    show_help
fi

eval set -- "$OPTIONS"

EXTRACT=false
BUILD=false

## Evaluate options
while true; do
    case "$1" in
        -e | --extract)
            EXTRACT=true
            shift
            ;;
        -b | --build)
            BUILD=true
            shift
            ;;
        -h | --help)
            show_help
            ;;
        --)
            shift
            break
            ;;
        *)
            show_help
            ;;
    esac
done

## Show help if no option was selected
if [ "$EXTRACT" = false ] && [ "$BUILD" = false ]; then
  show_help
fi

# --------------------------------------------------------------------------------
# Extract and .po initialisation (--extract)
# --------------------------------------------------------------------------------

if [ "$EXTRACT" = true ]; then
  mkdir -p locale/

  echo "Extract Strings from slint-ui (.slint).."

  slint-tr-extractor ui/**/*.slint -o locale/ui_strings.pot

  echo "Combine with C++ code..."

  xgettext  --keyword=gettext \
            --language=C++ \
            --from-code=UTF-8 \
            --join-existing \
            -o locale/ui_strings.pot \
            src/**/*.cpp include/**/*.hpp

  echo "Create final translation template..."

  mv locale/ui_strings.pot locale/updater.pot

  if [ ! -f locale/en.po ]; then
    echo "No existing english translation, initializing..."
    msginit --input=locale/updater.pot --locale=en_US.UTF-8 -o locale/en.po
  else
    echo "Existing english translation found, updating..."
    msgmerge --update locale/en.po locale/updater.pot
  fi

  if [ ! -f locale/de.po ]; then
    echo "No existing german translation, initializing..."
    msginit --input=locale/updater.pot --locale=de_DE.UTF-8 -o locale/de.po
  else
    echo "Existing german translation found, updating..."
    msgmerge --update locale/de.po locale/updater.pot
  fi

  echo "Done extracting"
fi

# --------------------------------------------------------------------------------
# Compile into .mo (--build)
# --------------------------------------------------------------------------------

if [ "$BUILD" = true ]; then
  echo "Compile into binary .mo files for runtime..."

  mkdir -p locale/en/LC_MESSAGES
  msgfmt locale/en.po -o locale/en/LC_MESSAGES/updater.mo

  mkdir -p locale/de/LC_MESSAGES
  msgfmt locale/de.po -o locale/de/LC_MESSAGES/updater.mo

  echo "Done building"
fi