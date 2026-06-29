#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FOCUSFUSION_D_LIB)
#  define FOCUSFUSION_D_EXPORT Q_DECL_EXPORT
# else
#  define FOCUSFUSION_D_EXPORT Q_DECL_IMPORT
# endif
#else
# define FOCUSFUSION_D_EXPORT
#endif
