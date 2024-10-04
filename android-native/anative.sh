#!/bin/bash
set -e

BLUE='\033[0;34m'
NC='\033[0m' # No Color

CHROMIUM_SRC_DIR=/media/abhijeet/Chromium-Ubuntu/wolvic/chromium/src
OUT_DIR=out/android-arm64-rel
WOLVIC_DIR=/media/abhijeet/Chromium-Ubuntu/wolvic/wolvic_app
WHERE_PREBUILT_AARS_ARE=/media/abhijeet/Chromium-Ubuntu/wolvic/prebuild
FILE="${OUT_DIR}/args.gn"

# Use sed to replace 'anative_build=false' with 'anative_build=true'
sed -i 's/anative_build=false/anative_build=true/' "$FILE"
cat $FILE

echo -e "${BLUE}Building chromium${NC}"

cd ${CHROMIUM_SRC_DIR}
gn gen ${OUT_DIR} && autoninja -C ${OUT_DIR} content/shell/android:content_shell_apk_anative -j 100
third_party/android_sdk/public/platform-tools/adb logcat --clear
echo -e "\r${BLUE}Build completed...${NC}"

echo -e "\rDONE"
