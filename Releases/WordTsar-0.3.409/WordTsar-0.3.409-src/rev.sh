#!/bin/bash

# increment revision (build) number
cd ..

MAJOR=0
MINOR=3

COUNTER_FILE="buildnum.txt"
OLD_BUILD_NUM=$(cat $COUNTER_FILE)
NEW_BUILD_NUM=$((OLD_BUILD_NUM+1))

YEAR=$(date +%Y)
SMALL_YEAR=$(date +%y)
MONTH=$(date +%m)
DAY=$(date +%d)


echo "#ifndef VERSION_H" > src/core/include/version.h
echo "#define VERSION_H" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "//Date Version Types" >> src/core/include/version.h
echo "static const char VDATE[] = \"$DAY\";" >> src/core/include/version.h
echo "static const char VMONTH[] = \"$MONTH\";" >> src/core/include/version.h
echo "static const char VYEAR[] = \"$YEAR\";" >> src/core/include/version.h
echo "static const char UBUNTU_VERSION_STYLE[] =  \"$SMALL_YEAR.$MONTH\";" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "//Software Status" >> src/core/include/version.h
echo "static const char STATUS[] =  \"pre-Alpha\";" >> src/core/include/version.h
echo "static const char STATUS_SHORT[] =  \"a\";" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "//Standard Version Type" >> src/core/include/version.h
echo "static const long MAJOR  = $MAJOR;" >> src/core/include/version.h
echo "static const long MINOR  = $MINOR;" >> src/core/include/version.h
echo "static const long BUILD  = $NEW_BUILD_NUM;" >> src/core/include/version.h
#echo "static const long REVISION  = $NEW_BUILD_NUM;" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "static const char FULLVERSION_STRING [] = \"$MAJOR.$MINOR build $NEW_BUILD_NUM\";" >> src/core/include/version.h
echo "" >> src/core/include/version.h
echo "#endif //VERSION_H" >> src/core/include/version.h
echo "" >> src/core/include/version.h

echo $NEW_BUILD_NUM > buildnum.txt

echo BUILD NUMBER INCREMENTED FROM $OLD_BUILD_NUM to $NEW_BUILD_NUM

