#ifndef EXPORT_IMPORT_HEADER_H
#define EXPORT_IMPORT_HEADER_H

#ifdef EXPORTING
#define DLL_SHARED __declspec( dllexport )
#else
#define DLL_SHARED __declspec( dllimport )
#endif

#endif