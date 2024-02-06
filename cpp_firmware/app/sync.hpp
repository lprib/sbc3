#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
namespace sync {

struct mutex {
   mutex() : m_semaphore(xSemaphoreCreateMutexStatic(&m_static_semaphore)) {}
   void lock() {
      (void)xSemaphoreTake(m_semaphore, portMAX_DELAY);
   }
   void unlock() {
      (void)xSemaphoreGive(m_semaphore);
   }

private:
   SemaphoreHandle_t m_semaphore = nullptr;
   StaticSemaphore_t m_static_semaphore;
private:
};

struct lock {
   lock(mutex& mutex) : m_mutex(mutex) {
      m_mutex.lock();
   }
   ~lock() {
      m_mutex.unlock();
   }

private:
   mutex& m_mutex;
};

struct scheduler_lock {
   scheduler_lock() {
      vTaskSuspendAll();
   }
   ~scheduler_lock() {
      if(!xTaskResumeAll()) {
         taskYIELD();
      }
   }
};
} // namespace sync
