#ifndef SAFEDATA_HEADER
#define SAFEDATA_HEADER
#include <list>
#include "SemaphoreClass.h"
#include "LockClass.h"

template <class T>
class SafeData {
	public:
		SafeData ( ) {
			m_list.clear ( );
		}
		~SafeData ( ) {
			m_list.clear ( );
		}
		int wait ( int timeout ) {
			struct timespec ts;

			clock_gettime(CLOCK_REALTIME, &ts);
	        ts.tv_sec += timeout;
	
			return m_sem.timedWait ( ts );
		}
		void signal ( ) {
			m_sem.post ( );
		}

		void lock ( ) {
			m_lock.lock ( );
		}
		void unlock ( ) {
			m_lock.unlock ( );
		}

		void push ( T new_entry ) {
			/*
				Safe Data
			*/
			lock ( );
			m_list.push_back ( new_entry );
			unlock ( );
		}
		T pop ( ) {
			/*
				Safe Data
			*/
			lock ( );
			T tmp = m_list.front ( );
			m_list.pop_front ( );
			unlock ( );
			return tmp;
		}

	private:
		std::list<T> m_list;
		SemClass m_sem;
		LockClass m_lock;
};

#endif