#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FLATNESS3D_LIB)
#  define FLATNESS3D_EXPORT Q_DECL_EXPORT
# else
#  define FLATNESS3D_EXPORT Q_DECL_IMPORT
# endif
#else
# define FLATNESS3D_EXPORT
#endif
