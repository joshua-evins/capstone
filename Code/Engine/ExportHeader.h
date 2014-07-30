#ifndef EXPORTHEADER_H
#define EXPORTHEADER_H

#ifdef	ENGINE_EXPORTS
#define ENGINE_SHARED __declspec( dllexport )
#else
#define ENGINE_SHARED __declspec( dllimport )
#endif

#endif // EXPORTHEADER_H