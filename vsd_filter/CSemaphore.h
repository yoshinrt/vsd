/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CSemaphore.h - semaphore
	
*****************************************************************************/

#pragma once 

/*** セマフォ操作クラス *****************************************************/

class CSemaphore {
	
  public:
	CSemaphore(){
		m_hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );
	}
	
	~CSemaphore(){
		Lock();
		Release();
		CloseHandle( m_hSemaphore );
	}
	
	void Lock( void ){
		DebugMsgD( "Semaphore%08X: waiting\n", this );
		WaitForSingleObject( m_hSemaphore, INFINITE );
		DebugMsgD( "Semaphore%08X: entering critical section\n", this );
	}
	
	void Release( void ){
		DebugMsgD( "Semaphore%08X: exiting critical section\n", this );
		ReleaseSemaphore( m_hSemaphore, 1, NULL );
	}
	
  private:
	HANDLE m_hSemaphore;
};

class CSemaphoreLock {
	
  public:
	CSemaphoreLock( CSemaphore *pSemaphore ) : m_pSemaphore( pSemaphore ){
		m_pSemaphore->Lock();
	}
	
	~CSemaphoreLock(){
		m_pSemaphore->Release();
	}
	
  private:
	CSemaphore *m_pSemaphore;
};
