#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FINDSLOT_LIB)
#  define FINDSLOT_EXPORT Q_DECL_EXPORT
# else
#  define FINDSLOT_EXPORT Q_DECL_IMPORT
# endif
#else
# define FINDSLOT_EXPORT
#endif
