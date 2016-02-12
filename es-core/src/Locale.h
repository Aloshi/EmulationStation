#ifndef _LOCALE_H_
#define _LOCALE_H_

#include <boost/locale.hpp>

#define _(A) boost::locale::gettext(A)
#define _n(A, B, C) boost::locale::ngettext(A, B, C)

#endif
