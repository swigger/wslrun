#pragma once


int childrun(int (*f)(void*), void * mem, int len);

template <class T>
inline int childrun(int (*f)(T *), T* block)
{
	static_assert(__is_pod(T), "must be pod");
	return childrun( (int (*)(void*) ) f, (void*)block, sizeof(T));
}

#ifdef _WIN32
extern void * g_xblock;
extern int (*g_xmain)(void*);
extern void childrun_runhooks();
extern bool childrun_addhook_i(void (*f)(const char*, size_t), const char* str, size_t slen);

template <class R, class T1, class T2>
inline bool childrun_addhook(R(*f)(T1, T2), T1 value, T2 v2)
{
	string so;
	ISLStream * s = MakeStringSLStream(so);
	serialize(s, (void*)f);
	serialize(s, value);
	serialize(s, v2);
	delete s;

	struct LocalVR{
		static void run_it(const char * v, size_t len)
		{
			string so2(v, len);
			ISLStream * s2 = MakeStringSLStream(so2);
			R(*f)(T1,T2);
			T1 t1;
			T2 t2;
			if (unserialize(s2, (void*&)f) && unserialize(s2, t1) && unserialize(s2, t2))
			{
				f(t1, t2);
			}
		}
	};
	return childrun_addhook_i(&LocalVR::run_it, so.data(), so.length());
}

template <class R, class T1>
inline bool childrun_addhook(R(*f)(T1), T1 value)
{
	string so;
	ISLStream * s = MakeStringSLStream(so);
	serialize(s, (void*)f);
	serialize(s, value);
	delete s;

	struct LocalVR{
		static void run_it(const char * v, size_t len)
		{
			string so2(v, len);
			ISLStream * s2 = MakeStringSLStream(so2);
			R(*f)(T1);
			T1 t1;
			if (unserialize(s2, (void*&)f) && unserialize(s2, t1))
			{
				f(t1);
			}
		}
	};
	return childrun_addhook_i(&LocalVR::run_it, so.data(), so.length());
}

template <class R>
inline bool childrun_addhook(R(*f)())
{
	static_assert(sizeof(R) <= 8 && __is_pod(R), "DO not return large obj!");
	return childrun_addhook_i( (void(*)(const char*, size_t))f, "", 0);
}
template <>
inline bool childrun_addhook(void(*f)())
{
	return childrun_addhook_i((void(*)(const char*, size_t))f, "", 0);
}

#define CHILDRUN_HOOK_MAIN()					\
	do { if (g_xmain) {							\
		childrun_runhooks();					\
		int ix = g_xmain(g_xblock);				\
		VirtualFree(g_xblock, 0, MEM_RELEASE);	\
		return ix;								\
	} }while(0)

#else
#define CHILDRUN_HOOK_MAIN()
#define childrun_addhook(...)
#endif

inline void inherit_handler(HANDLE h)
{
#ifdef _WIN32
	SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
#endif
}

inline void inherit_socket(int sock)
{
	inherit_handler((HANDLE)(intptr_t)sock);
}
