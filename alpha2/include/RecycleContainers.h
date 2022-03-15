template<typename T>
class PointerRecycler
{
   private:
      const std::string fRecyclerName;
      const int fMaxBufferedEvents;
      std::deque<T*> fRecycleBin;
      int fNEventsRecycled;
      int fNEventsCreatedNew;
      int fNEventsThrownAway;
      int fNEventsQueued;
   private:
      T* GrabObject()
      {
         T* newObject = fRecycleBin.front();
         fRecycleBin.pop_front();
         newObject->Reset();
         return newObject;
      }
   public:
      PointerRecycler(const int buffer_events, const char* nick_name):
         fRecyclerName(nick_name),
         fMaxBufferedEvents(buffer_events)
      {
         fNEventsRecycled = 0;
         fNEventsCreatedNew = 0;
         fNEventsThrownAway = 0;
         fNEventsQueued = 0;
      }
      ~PointerRecycler()
      {
          std::cout << fRecyclerName.c_str() << " PointerRecycler\n";
          std::cout << "\t" << fNEventsRecycled << " events recycled\n";
          std::cout << "\t" << fNEventsCreatedNew << " events created\n";
          std::cout << "\t" << fNEventsThrownAway << " events thrown away (queue full)\n";
          std::cout << "\t" << fNEventsQueued << " events queued\n";
          for (T* p: fRecycleBin)
             delete p;
          fRecycleBin.clear();
      }
      T* NewObject()
      {
         if (fRecycleBin.size())
         {
            fNEventsRecycled++;
            return GrabObject();
         }
         else
         {
            fNEventsCreatedNew++;
            return new T();
         }
      }
      void RecycleObject(T* data)
      {
         if (fRecycleBin.size() < fMaxBufferedEvents)
         {
            fNEventsQueued++;
            fRecycleBin.push_back(data);
         }
         else
         {
            fNEventsThrownAway++;
            delete data;
         }
      }
};

class VectorRecyclerTableWriter
{
   public:
   VectorRecyclerTableWriter() {
      std::cout <<"PLOOP TABLE"<<std::endl;
   };
   ~VectorRecyclerTableWriter()
   {
      std::cout <<"MY TABLE"<<std::endl;
   };
};

#define ENABLE_VECTOR_RECYCLING 0

template<typename T>
class VectorRecycler
{
   private:
      const std::string fRecyclerName;
      const int fMaxBufferedEvents;
      std::deque<std::vector<T>> fRecycleBin;
      std::mutex fMutex;

      int fNEventsRecycled;
      int fNEventsCreatedNew;
      int fNEventsThrownAway;
      int fNEventsQueued;

      int fMemInUse;
      int fMaxMemUse;

   private:
      std::vector<T> GrabVector()
      {
         while (fRecycleBin.front().capacity() == 0)
         {
            fRecycleBin.pop_front();
            if (fRecycleBin.empty())
               break;
         }
         if (fRecycleBin.size())
         {
            fNEventsRecycled++;
            std::vector<T> data = std::move(fRecycleBin.front());
            fMemInUse -= data.capacity();
            return data;
         }
         else
         {
            fNEventsCreatedNew++;
            return std::vector<T>();
         }
      }
   public:
      VectorRecycler(const int buffer_events, const char* nick_name):
         fRecyclerName(nick_name),
         fMaxBufferedEvents(buffer_events)
      {

      }
      ~VectorRecycler()
      {
#if ENABLE_VECTOR_RECYCLING
          std::cout << fRecyclerName.c_str() << " Container Recycler\n";
          std::cout << "\t" << fNEventsRecycled << " events recycled\n";
          std::cout << "\t" << fNEventsCreatedNew << " events created\n";
          std::cout << "\t" << fNEventsThrownAway << " events thrown away (queue full)\n";
          std::cout << "\t" << fNEventsQueued << " events queued\n";
          if (fMemInUse > 1024 * 1024)
          {
             std::cout << "\t" << fMemInUse / 1024 / 1024 << "MB mem in use\n";
          } else if (fMemInUse > 1024 ) {
             std::cout << "\t" << fMemInUse / 1024 << "kB mem in use\n";
          } else {
             std::cout << "\t" << fMemInUse << "B mem in use\n";
          }
          //std::cout << "\t" << fMemInUse << " mem in use ("<<fMaxMemUse<<")\n";
          fRecycleBin.clear();
#endif
      }
      std::vector<T> NewVector()
      {
#if ENABLE_VECTOR_RECYCLING
         std::lock_guard<std::mutex> guard(fMutex);
         if (fRecycleBin.size())
            return GrabVector();
         else
#endif
            return std::vector<T>();
      }
      void RecycleVector(std::vector<T> data)
      {
#if ENABLE_VECTOR_RECYCLING
         std::lock_guard<std::mutex> guard(fMutex);
         // Dont recycle empty containers
         if (!data.capacity())
            return;
         if (fRecycleBin.size() < fMaxBufferedEvents)
         {
            fNEventsQueued++;
            data.clear();
            fMemInUse += data.capacity();
            fRecycleBin.emplace_back(std::move(data));
         }
         else
         {
            //std::cout << "VECTOR BUFFER FULL!" << typeid(T).name()<<std::endl;
            if (fMemInUse > fMaxMemUse) 
               fMaxMemUse = fMemInUse;
            fNEventsThrownAway++;
            data.clear();
         }
#else 
         data.clear();
#endif
      }
};
