#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(GAP3D_LIB)
#  define GAP3D_EXPORT Q_DECL_EXPORT
# else
#  define GAP3D_EXPORT Q_DECL_IMPORT
# endif
#else
# define GAP3D_EXPORT
#endif
