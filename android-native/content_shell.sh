#!/bin/bash
set -e

BLUE='\033[0;34m'
NC='\033[0m' # No Color

CHROMIUM_SRC_DIR=/media/abhijeet/Chromium-Ubuntu/wolvic/chromium/src
OUT_DIR=out/android-arm64-rel
WOLVIC_DIR=/media/abhijeet/Chromium-Ubuntu/wolvic/wolvic_app
WHERE_PREBUILT_AARS_ARE=/media/abhijeet/Chromium-Ubuntu/wolvic/prebuild
FILE="${OUT_DIR}/args.gn"

# Use sed to replace 'anative_build=true' with 'anative_build=false'
sed -i 's/anative_build=true/anative_build=false/' "$FILE"
cat $FILE

echo -e "${BLUE}Building chromium${NC}"

cd ${CHROMIUM_SRC_DIR}
third_party/android_sdk/public/platform-tools/adb logcat --clear
gn gen ${OUT_DIR} && autoninja -C ${OUT_DIR} content/shell/android:content_shell_apk -j 100

echo -e "\r${BLUE}Build completed...${NC}"

echo -e "\rDONE"
