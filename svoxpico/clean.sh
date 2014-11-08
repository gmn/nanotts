
function EE() {
    echo "$@"
    $@
}

EE "rm -rf .deps Makefile Makefile.in aclocal.m4 compile config.guess config.log config.status config.sub configure depcomp install-sh libtool ltmain.sh m4 missing autom4te.cache"

