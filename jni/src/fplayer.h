#ifndef FPLAYER
#define FPLAYER

#ifdef __cplusplus
#else
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC__) || defined(__cplusplus)

extern int start_engine();
extern int shutdown_engine();
extern int start_audio_stream(char* filename);

#else

extern int start_engine();
extern int shutdown_engine();
extern int start_audio_stream(char*);

#endif

#ifdef __cplusplus
}
#endif

#endif /*FPLAYER*/