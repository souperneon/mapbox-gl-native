#!/usr/bin/env bash

set -e
set -o pipefail
set -u

xctool test \
    -project ./test/ios/ios-tests.xcodeproj \
    -scheme 'Mapbox GL Tests' \
    -sdk iphonesimulator \
    -destination 'platform=iOS Simulator,name=iPhone 5s,OS=7.1' \
    -destination 'platform=iOS Simulator,name=iPhone 5s,OS=8.1' \
    -destination 'platform=iOS Simulator,name=iPad 2,OS=7.1' \
    -destination 'platform=iOS Simulator,name=iPad 2,OS=8.1'
