# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

include $(top_builddir)/version.mk

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
EXTRA_DIST += $(man1_MANS)
SUFFIXES =

man1_MANS = $(built_men)

built_men =
built_men += terms.man
built_men += enum.man
built_men += glep.man
built_men += ldmatrix.man
built_men += uncol.man
built_men += qgscore.man

BUILT_SOURCES += $(built_men)
EXTRA_DIST += author.h2m

VPATH += @top_srcdir@/src
SUFFIXES += .yuck
SUFFIXES += .man
.yuck.man:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) genman \
			--package "$(PACKAGE_NAME)" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-file "$(top_builddir)/.version" $<

## Makefile.am ends here
