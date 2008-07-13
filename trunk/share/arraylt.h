/**
 *  arraylt.h: dynamic arrays
 *
 *  Copyright (C) 2008  Altap s.r.o.
 *
 *  You can not use this file or redistributed it without a permission from 
 *  Altap s.r.o.
 *
 */

// "light" verze array.h
// POZNAMKY:
// - sablony poli maji temer vsechny potrebne operace, proto ale jeste neni jejich pouziti
//   mrhani mistem v DLL modulu (linkuji se jen metody, ktere pouzijete, obvykle 500-1024 bytu)
// - u TDirectArray je dobre pouzivat jen jeden vybrany typ pro skupinu poli se stejne velikymi
//   prvky (napr. int, DWORD, (void *) a dalsi 4 bytove typy muzou pro usporu mista v DLL
//   pouzivat jen TDirectArray<DWORD>, ktere je navic predek TIndirectArray);
//   u TIndirectArray je jina situace, protoze metody jsou dost "tenke" (jen pretypovani)

#ifndef __ARRAYLT_H
#define __ARRAYLT_H

// makro _DEBUG slouzi k zapojeni kodu, ktery kontroluje ruzne chyby vznikle
// spatnym pouzivanim (viz CErrorType)

// aby byly arraye autonomni -> umi spolupracovat s TRACE, ale ted uz nepovinne
#if !defined(TRACE_I) && !defined(TRACE_E)
inline void __TraceEmptyFunction() {}
#define TRACE_I(str) __TraceEmptyFunction()
#define TRACE_E(str) __TraceEmptyFunction()
#endif // TRACE

enum CArrayDirection
{
  drUp,
  drDown
};

enum CDeleteType
{
  dtNoDelete,        // nevola se delete pro ukazatele z arraye
  dtDelete           // vola se delete pro ukazatele z arraye
};

enum CErrorType
{
  etNone,             // OK
  etLowMemory,        // new - NULL
  etUnknownIndex,     // mimo array
  etBadMove,          // posun prvku mimo array
  etBadDelete,        // mazani prvku, ktery neni v array
  etBadDeleteAt,      // mazani prvku, ktery neni v array
  etBadInsert,        // vkladani prvku mimo array
  etBadDetach,        // odpojeni prvku mimo array
  etBadDispose,       // mazani prvku, ktery neni v array
  etDestructed,       // po uspesnem volani Destroy()
};

// ****************************************************************************
// TDirectArray:
//  -chova se jako klasicke pole, navic se umi prealokovat na vetsi nebo mensi
//   podle potreby (podle hodnot 'base' a 'delta' viz konstruktor)
//  -pri mazani prvku z pole se vola metoda Destructor(index_prvku),
//   ktera v zakladnim objektu nic neprovadi

template <class DATA_TYPE>
class TDirectArray
{
  public:
    CErrorType State;                          // neni-li etNone stala se chyba
    int Count;                            // soucasny pocet polozek v kolekci

    TDirectArray<DATA_TYPE>(int base, int delta);
    virtual ~TDirectArray() { Destroy(); }

    BOOL IsGood() const { return State == etNone; }
    void ResetState() { State = etNone; TRACE_I("Nulovani stavu pole (TDirectArray)."); }

    virtual void Destructor(int) {}

    void Insert(int index, const DATA_TYPE &member);
    int Add(const DATA_TYPE &member); // prida prvek na konec Arraye, vraci index prvku

    void Insert(int index, const DATA_TYPE *members, int count); // vlozeni count prvku members
    int Add(const DATA_TYPE *members, int count); // pridani count prvku members

    DATA_TYPE &At(int index)            // vraci ukazatel na prvek na pozici
    {
#ifdef _DEBUG
      if (index >= 0 && index < Count)
#endif
        return Data[index];
#ifdef _DEBUG
      else
      {
        TRACE_E("Pristup na neplatny prvek pole (index = " << index
                 << ", Count = " << Count << ").");
        Error(etUnknownIndex);
        return Data[0];  // kvuli kompileru vracim mozna neplatny prvek
      }
#endif
    }

    DATA_TYPE &operator [](int index)    // vraci ukazatel na prvek na pozici
    {
#ifdef _DEBUG
      if (index >= 0 && index < Count)
#endif
        return Data[index];
#ifdef _DEBUG
      else
      {
        TRACE_E("Pristup na neplatny prvek pole (index = " << index
                 << ", Count = " << Count << ").");
        Error(etUnknownIndex);
        return Data[0];  // kvuli kompileru vracim mozna neplatny prvek
      }
#endif
    }

    void DetachArray() { Data = NULL; State = etDestructed; Count = 0; Available = 0; }
    DATA_TYPE *GetData() { return Data; }

    void DestroyMembers();             // uvolni z pameti jen prvky, pole necha
    void DetachMembers();              // odpoji vsechny prvky, pole necha
    void Destroy();                              // kompletni destrukce objektu
    void Delete(int index);         // zrusi prvek na pozici a ostatni posune
    void Delete(int index, int count);  // zrusi count prvku od pozice a ostatni posune
    void Detach(int index);         // odpoji prvek na pozici a ostatni posune
    void Detach(int index, int count);  // odpoji count prvku od pozice a ostatni posune

    int SetDelta(int delta);        // doporucuje 'Delta', vraci realne nastavenou hodnotu; lze volat jen pro prazdne pole

  protected:
    DATA_TYPE *Data;                                        // ukazatel na pole
    int Available;                         // na kolik vystaci soucastny stav
    int Base;                            // kolik polozek je v zakladnim poli
    int Delta;                             // kolik polozek ma mit dalsi pole

    virtual void Error(CErrorType err)            // zpracovani chyby v kolekci
    {
      if (State == etNone)
        State = err;
      else
        TRACE_E("Chybne volani metody Error (State = " << State << ").");
    }
    void EnlargeArray();                                    // zvetsi nove pole
    void ReduceArray();                                          // zmensi pole

    void Move(CArrayDirection direction, int first, int count);
                                                                  // posune o 1
  private:                    // nasledujici metody se nebudou volat (prevence)
    TDirectArray<DATA_TYPE>() {}
    TDirectArray<DATA_TYPE>(const TDirectArray<DATA_TYPE> &) {}
    TDirectArray<DATA_TYPE> &operator =(TDirectArray<DATA_TYPE> &) { return *this; }
};

// ****************************************************************************
// CArray:
//  -predek vsech indirect poli
//  -drzi typ (void *) v DWORD poli (kvuli uspore mista v .exe)

class CArray: public TDirectArray<DWORD>
{
  protected:
    CDeleteType DeleteType;

  public:
    CArray(int base, int delta, CDeleteType dt): TDirectArray<DWORD>(base, delta)
    {
      DeleteType = dt;
    }
};

// ****************************************************************************
// TIndirectArray:
//  -vhodne pro ulozeni ukazatelu na objekty
//  -ostatni vlastnosti viz CArray

template <class DATA_TYPE>
class TIndirectArray: public CArray
{
  public:
    TIndirectArray(int base, int delta, CDeleteType dt = dtDelete)
                                                   : CArray(base, delta, dt) {}
    DATA_TYPE *&At(int index)
    {
      return (DATA_TYPE *&)(CArray::operator [](index));
    }

    DATA_TYPE *&operator [](int index)
    {
      return (DATA_TYPE *&)(CArray::operator [](index));
    }

    void Detach(int index)
    {
      CArray::Detach(index);
    }

    void Insert(int index, DATA_TYPE *member)
    {
      CArray::Insert(index, (DWORD)member);
    }

    int Add(DATA_TYPE *member)
    {
      return CArray::Add((DWORD)member);
    }

    void Insert(int index, DATA_TYPE * const *members, int count)
    {
      CArray::Insert(index, (DWORD *)members, count);
    }

    int Add(DATA_TYPE * const *members, int count)
    {
      return CArray::Add((DWORD *)members, count);
    }

    DATA_TYPE **GetData()
    {
      return (DATA_TYPE **)Data;
    }

    virtual ~TIndirectArray() { Destroy(); }

  protected:
    virtual void Destructor(int i)
    {
      void *member = (void *)Data[i];
      if (DeleteType == dtDelete && member != NULL)
        delete ((DATA_TYPE *)member);
    }
};

//
// ****************************************************************************
// TDirectArray
//

template <class DATA_TYPE>
TDirectArray<DATA_TYPE>::TDirectArray(int base, int delta)
{
  Base = (base > 0) ? base : 1;
  Delta = (delta > 0) ? delta : 1;
  State = etNone;
  Available = Count = 0;
  Data = (DATA_TYPE *)malloc(Base * sizeof(DATA_TYPE));
  if (Data == NULL)
  {
    TRACE_E("Nedostatek pameti.");
    Error(etLowMemory);
    return;
  }
  Available = Base;
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Destroy()
{
  if (State == etNone)  // muze prijit etDestructed
  {
    if (Data != NULL)
    {
      for (int i = 0; i < Count; i++)
        Destructor(i);
      free(Data);
      Data = NULL;
      State = etDestructed;
    }
  }
#ifdef _DEBUG
  else
    if (State != etDestructed)
      TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Insert(int index, const DATA_TYPE &member)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (Count == Available && &member >= Data && &member < Data + Count)
    {
      TRACE_E("Vkladany prvek mohl behem operace ztratit platnost.");
      return;
    }
    if (Count < Available &&
        &member >= Data + index + 1 && &member < Data + Count + 1)
      TRACE_E("Vkladany prvek zmeni behem vkladani svou hodnotu.");
#endif
    if (index >= 0 && index <= Count)
    {
      Move(drDown, index, Count - index);
      if (State == etNone)
      {
        Count++;
        At(index) = member;
      }
    }
    else
    {
      TRACE_E("Pokus o vlozeni prvku mimo pole.");
      State = etBadInsert;
    }
#ifdef _DEBUG
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int
TDirectArray<DATA_TYPE>::Add(const DATA_TYPE &member)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (Count == Available && &member >= Data && &member < Data + Count)
    {
      TRACE_E("Pridavany prvek mohl behem operace ztratit platnost.");
      return ULONG_MAX;
    }
#endif
    if (Count == Available)
      EnlargeArray();
    if (State == etNone)
    {
      Count++;
      At(Count - 1) = member;
      return Count - 1;
    }
#ifdef _DEBUG
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
  return ULONG_MAX;
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Insert(int index, const DATA_TYPE *members, int count)
{
  if (count <= 0) return;  // neni co delat
#ifdef _DEBUG
  if (State == etNone)
  {
    if (Count + count > Available && members + count - 1 >= Data &&  // bude se realokovat a vkladane prvky jsou z tohoto pole (po realokaci pole se tedy prvky uvoni)
        members < Data + Count)
    {
      TRACE_E("Vkladane prvky mohly behem operace ztratit platnost.");
      return;
    }
    if (Count + count <= Available &&                    // pokud se nebude realokovat pole
        members + count - 1 >= Data + index + count &&   // a pokud se vkladany usek naleza v oblasti, kam se rozesune pole
        members < Data + Count + count)
    {
      TRACE_E("Vkladane prvky zmeni behem vkladani svou hodnotu.");
    }
#endif
    if (index >= 0 && index <= Count)
    {
      int needed = Count + count;
      if (needed > Available)
      {
        needed -= Base + 1;
        needed = needed - (needed % Delta) + Delta + Base;
        DATA_TYPE *newData = (DATA_TYPE *)realloc(Data, needed * sizeof(DATA_TYPE));
        if (newData == NULL)
        {
          TRACE_E("Nedostatek pameti pro zvetseni pole.");
          Error(etLowMemory);
          return;
        }
        Available = needed;
        Data = newData;
      }
      memmove(Data + index + count, Data + index, (Count - index) * sizeof(DATA_TYPE));
      memmove(Data + index, members, count * sizeof(DATA_TYPE));
      Count += count;
    }
    else
    {
      TRACE_E("Pokus o vlozeni prvku mimo pole.");
      State = etBadInsert;
    }
#ifdef _DEBUG
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int
TDirectArray<DATA_TYPE>::Add(const DATA_TYPE *members, int count)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (Count + count > Available && members + count - 1 >= Data &&
        members < Data + Count)
    {
      TRACE_E("Pridavane prvky mohly behem operace ztratit platnost.");
      return ULONG_MAX;
    }
#endif
    int needed = Count + count;
    if (needed > Available)
    {
      needed -= Base + 1;
      needed = needed - (needed % Delta) + Delta + Base;
      DATA_TYPE *newData = (DATA_TYPE *)realloc(Data, needed * sizeof(DATA_TYPE));
      if (newData == NULL)
      {
        TRACE_E("Nedostatek pameti pro zvetseni pole.");
        Error(etLowMemory);
        return ULONG_MAX;
      }
      Available = needed;
      Data = newData;
    }
    memmove(Data + Count, members, count * sizeof(DATA_TYPE));
    Count += count;
    return Count - count;
#ifdef _DEBUG
  }
  return ULONG_MAX;
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::DestroyMembers()
{
#ifdef _DEBUG
  if (State == etNone)
  {
#endif
    if (Count > 0)
      for (int i = 0; i < Count; i++)
        Destructor(i);
    else
      return;
    Count = 0;
    if (Available == Base)
      return;

    DATA_TYPE *newData = (DATA_TYPE *)realloc(Data, Base * sizeof(DATA_TYPE));
    if (newData == NULL)
    {
      TRACE_E("Nedostatek pameti pro operace spojene se zrusenim prvku pole.");
      Error(etLowMemory);
      return;
    }
    Available = Base;
    Data = newData;
#ifdef _DEBUG
  }
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::DetachMembers()
{
#ifdef _DEBUG
  if (State == etNone)
  {
#endif
    if (Count == 0)
      return;
    Count = 0;
    if (Available == Base)
      return;

    DATA_TYPE *newData = (DATA_TYPE *)realloc(Data, Base * sizeof(DATA_TYPE));
    if (newData == NULL)
    {
      TRACE_E("Nedostatek pameti pro operace spojene se zrusenim prvku pole.");
      Error(etLowMemory);
      return;
    }
    Available = Base;
    Data = newData;
#ifdef _DEBUG
  }
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Delete(int index)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (index >= 0 && index < Count)
    {
#endif
      Destructor(index);
      Move(drUp, index + 1, Count - index - 1);
      Count--;
      if (Available > Base && Available - Delta == Count)
        ReduceArray();
#ifdef _DEBUG
    }
    else
      TRACE_E("Mazani mimo pole. index = " << index << ", count = " << Count);
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Delete(int index, int count)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (index >= 0 && index + count - 1 < Count)
    {
#endif
      for (int i = index; i < index + count; i++) Destructor(i);
      memmove(Data + index, Data + index + count, (Count - count - index) * sizeof(DATA_TYPE));
      Count -= count;
      if (Available > Base && Available - Delta >= Count)
      {
        int a = (Count <= Base) ? Base : Base + Delta * ((Count - Base - 1) / Delta + 1);
        DATA_TYPE *New = (DATA_TYPE *)realloc(Data, a * sizeof(DATA_TYPE));
        if (New == NULL)
        {
          TRACE_E("Nedostatek pameti pro operace spojene se zmensenim pole.");
          Error(etLowMemory);
        }
        else
        {
          Data = New;
          Available = a;
        }
      }
#ifdef _DEBUG
    }
    else
      TRACE_E("Mazani mimo pole. index = " << index << ", count = " << count << ", Count = " << Count);
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Detach(int index)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (index >= 0 && index < Count)
    {
#endif
      Move(drUp, index + 1, Count - index - 1);
      Count--;
      if (Available > Base && Available - Delta == Count)
        ReduceArray();
#ifdef _DEBUG
    }
    else
      TRACE_E("Mazani mimo pole. index = " << index << ", count = " << Count);
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Detach(int index, int count)
{
#ifdef _DEBUG
  if (State == etNone)
  {
    if (index >= 0 && index + count - 1 < Count)
    {
#endif
      memmove(Data + index, Data + index + count, (Count - count - index) * sizeof(DATA_TYPE));
      Count -= count;
      if (Available > Base && Available - Delta >= Count)
      {
        int a = (Count <= Base) ? Base : Base + Delta * ((Count - Base - 1) / Delta + 1);
        DATA_TYPE *New = (DATA_TYPE *)realloc(Data, a * sizeof(DATA_TYPE));
        if (New == NULL)
        {
          TRACE_E("Nedostatek pameti pro operace spojene se zmensenim pole.");
          Error(etLowMemory);
        }
        else
        {
          Data = New;
          Available = a;
        }
      }
#ifdef _DEBUG
    }
    else
      TRACE_E("Mazani mimo pole. index = " << index << ", count = " << count << ", Count = " << Count);
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
int
TDirectArray<DATA_TYPE>::SetDelta(int delta)
{
#ifdef _DEBUG
  if (State == etNone)
  {
#endif // _DEBUG
    if (Count == 0)
    {
      if (delta > 0 && delta > Delta)
        Delta = delta;
    }
#ifdef _DEBUG
    else
      TRACE_E("SetDelta() lze volat pouze pro prazdne pole. (Count = " << Count << ").");
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif // _DEBUG
  return Delta;
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::EnlargeArray()
{
#ifdef _DEBUG
  if (State == etNone)
  {
#endif
    DATA_TYPE *New = (DATA_TYPE *)realloc(Data, (Available + Delta) * sizeof(DATA_TYPE));
    if (New == NULL)
    {
      TRACE_E("Nedostatek pameti pro zvetseni pole.");
      Error(etLowMemory);
      return;
    }
    Data = New;
    Available += Delta;
#ifdef _DEBUG
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::ReduceArray()
{
#ifdef _DEBUG
  if (State == etNone)
  {
#endif
    DATA_TYPE *New = (DATA_TYPE *)realloc(Data, (Available - Delta) * sizeof(DATA_TYPE));
    if (New == NULL)
    {
      TRACE_E("Nedostatek pameti pro operace spojene se zmensenim pole.");
      Error(etLowMemory);
      return;
    }
    Data = New;
    Available -= Delta;
#ifdef _DEBUG
  }
  else
    TRACE_E("Chybne volani metody pole (State = " << State << ").");
#endif
}

template <class DATA_TYPE>
void
TDirectArray<DATA_TYPE>::Move(CArrayDirection direction, int first, int count)
{
  if (count == 0)
  {
    if (direction == drDown && Available == Count)
      EnlargeArray();
    return;
  }
  if (direction == drDown)
  {
    if (Available == Count)
      EnlargeArray();
    if (State == etNone)
      memmove(Data + first + 1, Data + first, count * sizeof(DATA_TYPE));
  }
  else  // Up
    memmove(Data + first - 1, Data + first, count * sizeof(DATA_TYPE));
}

#endif // __ARRAYLT_H
