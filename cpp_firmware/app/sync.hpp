#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
namespace sync {

struct mutex_t {
   mutex_t() : m_semaphore(xSemaphoreCreateMutexStatic(&m_static_semaphore)) {}
   void lock() {
      (void)xSemaphoreTake(m_semaphore, portMAX_DELAY);
   }
   void unlock() {
      (void)xSemaphoreGive(m_semaphore);
   }

private:
   SemaphoreHandle_t m_semaphore = nullptr;
   StaticSemaphore_t m_static_semaphore;
};

struct lock_t {
   lock_t(mutex_t& mutex) : m_mutex(mutex) {
      m_mutex.lock();
   }
   ~lock_t() {
      m_mutex.unlock();
   }

private:
   mutex_t& m_mutex;
};

struct scheduler_lock_t {
   scheduler_lock_t() {
      vTaskSuspendAll();
   }
   ~scheduler_lock_t() {
      if(!xTaskResumeAll()) {
         taskYIELD();
      }
   }
};

template <typename T> struct queue {
   queue(int capacity) :
      queue(
         capacity,
         static_cast<uint8_t*>(pvPortMalloc((size_t)capacity * sizeof(T)))
      ) {}
   queue(int capacity, uint8_t* buffer) :
      m_queue(xQueueCreateStatic(
         (size_t)capacity, sizeof(T), buffer, &m_static_queue
      )) {}

   void blocking_send(T item) {
      xQueueSendToBack(m_queue, &item, portMAX_DELAY);
   }

   bool isr_send(T item) {
      // BaseType_t higher_priority_task_woken;
      return xQueueSendToBackFromISR(m_queue, item, nullptr);
      // if(higher_priority_woken) {
      //    taskYIELD();
      // }
   }

   [[nodiscard]] T blocking_receive() {
      T item;
      xQueueReceive(m_queue, &item, portMAX_DELAY);
      return item;
   }

private:
   QueueHandle_t m_queue;
   StaticQueue_t m_static_queue;
};

} // namespace sync
