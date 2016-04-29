aclocal -I config
LIBTOOLIZE=`which libtoolize`
if [ $? == 1 ]; then
    LIBTOOLIZE=`which glibtoolize`
fi
if [ ! -z $LIBTOOLIZE ]; then
    $LIBTOOLIZE --force --copy
else
    echo "No libtoolize or glibtoolize found"
fi
autoconf
autoheader
automake --add-missing --copy
