#include "pch.h"

#include "CWndMain.h"

void CheckRangeEqual(ILVRange* p1, ILVRange* p2, int idxMax)
{
    LONG l1, l2;
    EckCounter(idxMax, i)
    {
        EckAssert(p1->IsSelected(i) == p2->IsSelected(i));
        p1->NextSelected(i, &l1);
        p2->NextSelected(i, &l2);
        EckAssert(l1 == l2);
        p1->NextUnSelected(i, &l1);
        p2->NextUnSelected(i, &l2);
        EckAssert(l1 == l2);
    }
    p1->CountIncluded(&l1);
    p2->CountIncluded(&l2);
    EckAssert(l1 == l2);
}

#define MINCOUNT 6      // number of sel ranges to start with amd maintain
#define GROWSIZE 150    // percent to grow when needed

#define COUNT_SELRANGES_NONE 2     // When count of selranges really means none

#define SELRANGE_MINVALUE  0
#define SELRANGE_MAXVALUE  LONG_MAX - 2
#define SELRANGE_ERROR      LONG_MAX

typedef struct tag_SELRANGEITEM
{
    LONG iBegin;
    LONG iEnd;
} SELRANGEITEM, * PSELRANGEITEM;


class CLVRange : public ILVRange

{
public:
    // *** IUnknown methods ***
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // *** ILVRange methods ***
    STDMETHODIMP IncludeRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP ExcludeRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP InvertRange(LONG iBegin, LONG iEnd);
    STDMETHODIMP InsertItem(LONG iItem);
    STDMETHODIMP RemoveItem(LONG iItem);

    STDMETHODIMP Clear();
    STDMETHODIMP IsSelected(LONG iItem);
    STDMETHODIMP IsEmpty();
    STDMETHODIMP NextSelected(LONG iItem, LONG* piItem);
    STDMETHODIMP NextUnSelected(LONG iItem, LONG* piItem);
    STDMETHODIMP CountIncluded(LONG* pcIncluded);

protected:
    // Helper Functions.
    friend ILVRange* LVRange_Create();
    CLVRange();
    ~CLVRange();

    BOOL        _Enlarge();
    BOOL        _Shrink();
    BOOL        _InsertRange(LONG iAfterItem, LONG iBegin, LONG iEnd);
    HRESULT     _RemoveRanges(LONG iStartItem, LONG iStopItem, LONG* p);
    BOOL        _FindValue(LONG Value, LONG* piItem);
    void        _InitNew();

    int           _cRef;
    PSELRANGEITEM _VSelRanges;  // Vector of sel ranges
    LONG          _cSize;       // size of above vector in sel ranges
    LONG          _cSelRanges;  // count of sel ranges used
    LONG          _cIncluded;   // Count of Included items...

    eck::CLVRange sr;
};

//-------------------------------------------------------------------
//
// Function: _Enlarge
//
// Summary:
//      This will enlarge the number of items the Sel Range can have.
//
// Arguments:
//      PSELRANGE [in]  - SelRange to Enlarge
//
// Return: FALSE if failed.
//
// Notes: Though this function may fail, pselrange structure is still valid
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL CLVRange::_Enlarge()
{
    LONG cNewSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = FALSE;


    cNewSize = _cSize * GROWSIZE / 100;
    pTempSelRange = (PSELRANGEITEM)GlobalReAlloc((HGLOBAL)_VSelRanges,
        cNewSize * sizeof(SELRANGEITEM),
        GMEM_ZEROINIT | GMEM_MOVEABLE);
    if (NULL != pTempSelRange)
    {
        _VSelRanges = pTempSelRange;
        _cSize = cNewSize;
        frt = TRUE;
    }
    return(frt);
}

//-------------------------------------------------------------------
//
// Function: _Shrink
//
// Summary:
//      This will reduce the number of items the Sel Range can have.
//
// Arguments:
//
// Return: FALSE if failed
//
// Notes: Shrink only happens when a significant size below the next size
//  is obtained and the new size is at least the minimum size.
//      Though this function may fail, pselrange structure is still valid
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL CLVRange::_Shrink()
{
    LONG cNewSize;
    LONG cTriggerSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = TRUE;


    // check if we are below last grow area by a small percent
    cTriggerSize = _cSize * 90 / GROWSIZE;
    cNewSize = _cSize * 100 / GROWSIZE;

    if ((_cSelRanges < cTriggerSize) && (cNewSize >= MINCOUNT))
    {
        pTempSelRange = (PSELRANGEITEM)GlobalReAlloc((HGLOBAL)_VSelRanges,
            cNewSize * sizeof(SELRANGEITEM),
            GMEM_ZEROINIT | GMEM_MOVEABLE);
        if (NULL != pTempSelRange)
        {
            _VSelRanges = pTempSelRange;
            _cSize = cNewSize;
        }
        else
        {
            frt = FALSE;
        }
    }
    return(frt);
}

//-------------------------------------------------------------------
//
// Function: _InsertRange
//
// Summary:
//      inserts a single range item into the range vector       
//
// Arguments:
//      iAfterItem [in] - Index to insert range after, -1 means insert as first item
//      iBegin [in]     - begin of range
//      iEnd [in]       - end of the range
//
// Return:
//      TRUE if succesful, otherwise FALSE
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL CLVRange::_InsertRange(LONG iAfterItem,
    LONG iBegin,
    LONG iEnd)
{
    LONG iItem;
    BOOL frt = TRUE;

    EckAssert(iAfterItem >= -1);
    EckAssert(iBegin >= SELRANGE_MINVALUE);
    EckAssert(iEnd >= iBegin);
    EckAssert(iEnd <= SELRANGE_MAXVALUE);
    EckAssert(_cSelRanges < _cSize);

    // shift all over one
    for (iItem = _cSelRanges; iItem > iAfterItem + 1; iItem--)
    {
        _VSelRanges[iItem] = _VSelRanges[iItem - 1];
    }
    _cSelRanges++;

    // make the insertion
    _VSelRanges[iAfterItem + 1].iBegin = iBegin;
    _VSelRanges[iAfterItem + 1].iEnd = iEnd;

    // make sure we have room next time
    if (_cSelRanges == _cSize)
    {
        frt = _Enlarge();
    }
    return(frt);
}

//-------------------------------------------------------------------
//
// Function: _RemoveRanges
//
// Summary:
//      Removes all ranged between and including the speicifed indexes      
//
// Arguments:
//      iStartItem [in] - Index to start removal
//      iStopItem [in]  - Index to stop removal
//
// Return:
//      SELRANGE_ERROR on memory allocation error
//      The number of items that are unselected by this removal
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

HRESULT CLVRange::_RemoveRanges(LONG iStartItem, LONG iStopItem, LONG* pc)
{
    LONG iItem;
    LONG diff;
    LONG cUnSelected = 0;
    HRESULT hres = S_OK;

    EckAssert(iStartItem > 0);
    EckAssert(iStopItem >= iStartItem);
    EckAssert(iStartItem < _cSelRanges - 1);
    EckAssert(iStopItem < _cSelRanges - 1);

    diff = iStopItem - iStartItem + 1;

    for (iItem = iStartItem; iItem <= iStopItem; iItem++)
        cUnSelected += _VSelRanges[iItem].iEnd -
        _VSelRanges[iItem].iBegin + 1;

    // shift all over the difference
    for (iItem = iStopItem + 1; iItem < _cSelRanges; iItem++, iStartItem++)
        _VSelRanges[iStartItem] = _VSelRanges[iItem];

    _cSelRanges -= diff;

    if (!_Shrink())
    {
        hres = E_FAIL;
    }
    else if (pc)
        *pc = cUnSelected;
    return(hres);
}


//-------------------------------------------------------------------
//
// Function: SelRange_FindValue
//
// Summary:
//      This function will search the ranges for the value, returning true
//  if the value was found within a range.  The piItem will contain the
//  the index at which it was found or the index before where it should be
//  The piItem may be set to -1, meaning that there are no ranges in the list
//      This functions uses a non-recursive binary search algorithm.
//
// Arguments:
//      piItem [out]    - Return of found range index, or one before
//      Value [in]      - Value to find within a range
//
// Return: True if found, False if not found
//
// Notes: The piItem will return one before if return is false.
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL CLVRange::_FindValue(LONG Value, LONG* piItem)
{
    LONG First;
    LONG Last;
    LONG Item;
    BOOL fFound = FALSE;

    EckAssert(piItem);
    EckAssert(_cSize >= COUNT_SELRANGES_NONE);
    EckAssert(Value >= SELRANGE_MINVALUE);
    EckAssert(Value <= SELRANGE_MAXVALUE);


    First = 0;
    Last = _cSelRanges - 1;
    Item = Last / 2;

    do
    {
        if (_VSelRanges[Item].iBegin > Value)
        {   // Value before this Item
            Last = Item;
            Item = (Last - First) / 2 + First;
            if (Item == Last)
            {
                Item = First;
                break;
            }
        }
        else if (_VSelRanges[Item].iEnd < Value)
        {   // Value after this Item
            First = Item;
            Item = (Last - First) / 2 + First;
            if (Item == First)
            {
                break;
            }
        }
        else
        {   // Value at this Item
            fFound = TRUE;
        }
    } while (!fFound);

    *piItem = Item;
    return(fFound);
}

//-------------------------------------------------------------------
//
// Function: _InitNew
//
// Summary:
//      This function will initialize a SelRange object.
//
// Arguments:
//
// Return:
//
// Notes:
//              
// History:
//      18-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

void CLVRange::_InitNew()
{
    _cSize = MINCOUNT;
    _cSelRanges = COUNT_SELRANGES_NONE;

    _VSelRanges[0].iBegin = LONG_MIN;
    // -2 and +2 below are to stop consecutive joining of end markers
    _VSelRanges[0].iEnd = SELRANGE_MINVALUE - 2;
    _VSelRanges[1].iBegin = SELRANGE_MAXVALUE + 2;
    _VSelRanges[1].iEnd = SELRANGE_MAXVALUE + 2;
    _cIncluded = 0;
}

//-------------------------------------------------------------------
//
// Function: SelRange_Create
//
// Summary:
//      This function will create and initialize a SelRange object.
//
// Arguments:
//
// Return: HSELRANGE that is created or NULL if it failed.
//
// Notes:
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

ILVRange* LVRange_Create()
{
    CLVRange* pselrange = new CLVRange;

    if (NULL != pselrange)
    {
        pselrange->_VSelRanges = (PSELRANGEITEM)GlobalAlloc(GPTR,
            sizeof(SELRANGEITEM) * MINCOUNT);
        if (NULL != pselrange->_VSelRanges)
        {
            pselrange->_InitNew();
        }
        else
        {
            delete pselrange;
            pselrange = NULL;
        }
    }

    return(pselrange ? pselrange : NULL);
}


//-------------------------------------------------------------------
//
// Function: Constructor
//
//-------------------------------------------------------------------
CLVRange::CLVRange()
{
    _cRef = 1;
}

//-------------------------------------------------------------------
//
// Function: Destructor
//
//-------------------------------------------------------------------
CLVRange::~CLVRange()
{
    GlobalFree(_VSelRanges);
}


//-------------------------------------------------------------------
//
// Function: QueryInterface
//
//-------------------------------------------------------------------
HRESULT CLVRange::QueryInterface(REFIID iid, void** ppv)
{
    if (IsEqualIID(iid, IID_ILVRange) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = static_cast<ILVRange*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    _cRef++;
    return NOERROR;
}

//-------------------------------------------------------------------
//
// Function: AddRef
//
//-------------------------------------------------------------------
ULONG CLVRange::AddRef()
{
    return ++_cRef;
}

//-------------------------------------------------------------------
//
// Function: Release
//
//-------------------------------------------------------------------
ULONG CLVRange::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}
HRESULT CLVRange::IncludeRange(LONG iBegin, LONG iEnd)
{
    sr.IncludeRange(iBegin, iEnd);
    LONG iFirst;   // index before or contains iBegin value
    LONG iLast;    // index before or contains iEnd value
    BOOL fExtendFirst;  // do we extend the iFirst or create one after it
    LONG iRemoveStart;  // start of ranges that need to be removed
    LONG iRemoveFinish; // end of ranges that need to be removed

    LONG iNewEnd;   // calculate new end value as we go
    BOOL fEndFound; // was the iEnd found in a range already
    BOOL fBeginFound; // was the iEnd found in a range already

    LONG cSelected = 0;
    HRESULT hres = S_OK;

    EckAssert(iEnd >= iBegin);
    EckAssert(iBegin >= SELRANGE_MINVALUE);
    EckAssert(iEnd <= SELRANGE_MAXVALUE);

    // find approximate locations
    fBeginFound = _FindValue(iBegin, &iFirst);
    fEndFound = _FindValue(iEnd, &iLast);


    //
    // Find First values
    //
    // check for consecutive End-First values
    if ((_VSelRanges[iFirst].iEnd == iBegin - 1) ||
        (fBeginFound))
    {
        // extend iFirst
        fExtendFirst = TRUE;
        iRemoveStart = iFirst + 1;
    }
    else
    {
        // create one after the iFirst
        fExtendFirst = FALSE;
        iRemoveStart = iFirst + 2;
    }

    //
    // Find Last values
    //
    if (fEndFound)
    {
        // Use [iLast].iEnd value
        iRemoveFinish = iLast;
        iNewEnd = _VSelRanges[iLast].iEnd;

    }
    else
    {
        // check for consecutive First-End values
        if (_VSelRanges[iLast + 1].iBegin == iEnd + 1)
        {
            // Use [iLast + 1].iEnd value
            iNewEnd = _VSelRanges[iLast + 1].iEnd;
            iRemoveFinish = iLast + 1;
        }
        else
        {
            // Use iEnd value
            iRemoveFinish = iLast;
            iNewEnd = iEnd;
        }
    }

    //
    // remove condenced items if needed
    //
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        hres = _RemoveRanges(iRemoveStart, iRemoveFinish, &cChange);
        if (FAILED(hres))
            return hres;
        else
        {
            cSelected -= cChange;
        }
    }

    //
    // insert item and reset values as needed
    //          
    if (fExtendFirst)
    {
        cSelected += iNewEnd - _VSelRanges[iFirst].iEnd;
        _VSelRanges[iFirst].iEnd = iNewEnd;
    }
    else
    {
        if (iRemoveStart > iRemoveFinish + 1)
        {
            cSelected += iEnd - iBegin + 1;
            // create one
            if (!_InsertRange(iFirst, iBegin, iNewEnd))
            {
                hres = E_FAIL;
            }
        }
        else
        {
            cSelected += iNewEnd - _VSelRanges[iFirst + 1].iEnd;
            cSelected += _VSelRanges[iFirst + 1].iBegin - iBegin;
            // no need to create one since the Removal would have left us one
            _VSelRanges[iFirst + 1].iEnd = iNewEnd;
            _VSelRanges[iFirst + 1].iBegin = iBegin;
        }
    }

    _cIncluded += cSelected;
    CheckRangeEqual(&sr, this, 9999);
    return(hres);
}

HRESULT CLVRange::ExcludeRange(LONG iBegin, LONG iEnd)
{
    sr.ExcludeRange(iBegin, iEnd);
    LONG iFirst;   // index before or contains iBegin value
    LONG iLast;    // index before or contains iEnd value
    LONG iRemoveStart;  // start of ranges that need to be removed
    LONG iRemoveFinish; // end of ranges that need to be removed

    LONG iFirstNewEnd;  // calculate new end value as we go
    BOOL fBeginFound; // was the iBegin found in a range already
    BOOL fEndFound;   // was the iEnd found in a range already
    LONG cUnSelected = 0;
    HRESULT hres = S_OK;

    EckAssert(iEnd >= iBegin);
    EckAssert(iBegin >= SELRANGE_MINVALUE);
    EckAssert(iEnd <= SELRANGE_MAXVALUE);

    // find approximate locations
    fBeginFound = _FindValue(iBegin, &iFirst);
    fEndFound = _FindValue(iEnd, &iLast);

    //
    // Find First values
    //

    // start removal after first
    iRemoveStart = iFirst + 1;
    // save FirstEnd as we may need to modify it
    iFirstNewEnd = _VSelRanges[iFirst].iEnd;

    if (fBeginFound)
    {
        // check for complete removal of first
        //    (first is a single selection or match?)
        if (_VSelRanges[iFirst].iBegin == iBegin)
        {
            iRemoveStart = iFirst;
        }
        else
        {
            // otherwise truncate iFirst
            iFirstNewEnd = iBegin - 1;
        }
    }

    //
    // Find Last values
    //

    // end removal on last
    iRemoveFinish = iLast;

    if (fEndFound)
    {
        // check for complete removal of last
        //   (first/last is a single selection or match?)
        if (_VSelRanges[iLast].iEnd != iEnd)
        {
            if (iFirst == iLast)
            {
                // split
                if (!_InsertRange(iFirst, iEnd + 1, _VSelRanges[iFirst].iEnd))
                {
                    return(E_FAIL);
                }
                cUnSelected -= _VSelRanges[iFirst].iEnd - iEnd;
            }
            else
            {
                // truncate Last
                iRemoveFinish = iLast - 1;
                cUnSelected += (iEnd + 1) - _VSelRanges[iLast].iBegin;
                _VSelRanges[iLast].iBegin = iEnd + 1;
            }
        }
    }

    // Now set the new end, since Last code may have needed the original values
    cUnSelected -= iFirstNewEnd - _VSelRanges[iFirst].iEnd;
    _VSelRanges[iFirst].iEnd = iFirstNewEnd;


    //
    // remove items if needed
    //
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        if (SUCCEEDED(hres = _RemoveRanges(iRemoveStart, iRemoveFinish, &cChange)))
            cUnSelected += cChange;
    }

    _cIncluded -= cUnSelected;
    CheckRangeEqual(&sr, this, 9999);
    return(hres);
}

HRESULT CLVRange::Clear()
{
    sr.Clear();
    PSELRANGEITEM pNewItems;
    HRESULT hres = S_OK;

    pNewItems = (PSELRANGEITEM)GlobalAlloc(GPTR,
        sizeof(SELRANGEITEM) * MINCOUNT);
    if (NULL != pNewItems)
    {
        GlobalFree(_VSelRanges);
        _VSelRanges = pNewItems;

        _InitNew();
    }
    else
    {
        hres = E_FAIL;
    }
    CheckRangeEqual(&sr, this, 9999);
    return(hres);
}

HRESULT CLVRange::IsSelected(LONG iItem)
{
    LONG iFirst;

    EckAssert(iItem >= 0);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    return(_FindValue(iItem, &iFirst) ? S_OK : S_FALSE);
}


//-------------------------------------------------------------------
//
// Function: SelRange_IsEmpty
//
// Summary:
//      This function will return TRUE if the range is empty
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//
// Return:  TRUE if empty
//
// Notes:
//
// History:
//
//-------------------------------------------------------------------
HRESULT CLVRange::IsEmpty()
{
    //CheckRangeEqual(&sr, this, 9999);
    return (_cSelRanges == COUNT_SELRANGES_NONE) ? S_OK : S_FALSE;
}

HRESULT CLVRange::CountIncluded(LONG* pcIncluded)
{
    *pcIncluded = _cIncluded;
    return S_OK;
}

HRESULT CLVRange::InsertItem(LONG iItem)
{
    sr.InsertItem(iItem);
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;

    EckAssert(iItem >= 0);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    if (_FindValue(iItem, &iFirst))
    {
        // split it
        if (_VSelRanges[iFirst].iBegin == iItem)
        {
            // but don't split if starts with value
            iFirst--;
        }
        else
        {
            if (!_InsertRange(iFirst, iItem, _VSelRanges[iFirst].iEnd))
            {
                return(E_FAIL);
            }
            _VSelRanges[iFirst].iEnd = iItem - 1;
        }
    }

    // now walk all ranges past iFirst, incrementing all values by one
    for (i = _cSelRanges - 2; i > iFirst; i--)
    {
        iBegin = _VSelRanges[i].iBegin;
        iEnd = _VSelRanges[i].iEnd;

        iBegin = std::min(SELRANGE_MAXVALUE, iBegin + 1);
        iEnd = std::min(SELRANGE_MAXVALUE, iEnd + 1);

        _VSelRanges[i].iBegin = iBegin;
        _VSelRanges[i].iEnd = iEnd;
    }
    CheckRangeEqual(&sr, this, 9999);
    return(S_OK);
}

HRESULT CLVRange::RemoveItem(LONG iItem)
{
    sr.RemoveItem(iItem);
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;
    HRESULT hres = S_OK;

    EckAssert(iItem >= SELRANGE_MINVALUE);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    if (_FindValue(iItem, &iFirst))
    {
        // item within, change the end value
        iEnd = _VSelRanges[iFirst].iEnd;
        iEnd = std::min(SELRANGE_MAXVALUE, iEnd - 1);
        _VSelRanges[iFirst].iEnd = iEnd;

        _cIncluded--;
    }
    else
    {
        // check for merge situation
        if ((iFirst < _cSelRanges - 1) &&
            (_VSelRanges[iFirst].iEnd == iItem - 1) &&
            (_VSelRanges[iFirst + 1].iBegin == iItem + 1))
        {
            _VSelRanges[iFirst].iEnd =
                _VSelRanges[iFirst + 1].iEnd - 1;
            if (FAILED(hres = _RemoveRanges(iFirst + 1, iFirst + 1, NULL)))
                return(hres);
        }
    }

    // now walk all ranges past iFirst, decrementing all values by one
    for (i = _cSelRanges - 2; i > iFirst; i--)
    {
        iBegin = _VSelRanges[i].iBegin;
        iEnd = _VSelRanges[i].iEnd;

        iBegin = std::min(SELRANGE_MAXVALUE, iBegin - 1);
        iEnd = std::min(SELRANGE_MAXVALUE, iEnd - 1);

        _VSelRanges[i].iBegin = iBegin;
        _VSelRanges[i].iEnd = iEnd;
    }
    CheckRangeEqual(&sr, this, 9999);
    return(hres);
}

HRESULT CLVRange::NextSelected(LONG iItem, LONG* piItem)
{
    LONG i;

    EckAssert(iItem >= SELRANGE_MINVALUE);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    if (!_FindValue(iItem, &i))
    {
        i++;
        if (i < _cSelRanges - 1)
        {
            iItem = _VSelRanges[i].iBegin;
        }
        else
        {
            iItem = -1;
        }
    }

    EckAssert(iItem >= -1);
    EckAssert(iItem <= SELRANGE_MAXVALUE);
    *piItem = iItem;
    return S_OK;
}

HRESULT CLVRange::NextUnSelected(LONG iItem, LONG* piItem)
{
    LONG i;

    EckAssert(iItem >= SELRANGE_MINVALUE);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    if (_FindValue(iItem, &i))
    {
        if (i < _cSelRanges - 1)
        {
            iItem = _VSelRanges[i].iEnd + 1;
            if (iItem > SELRANGE_MAXVALUE)
            {
                iItem = -1;
            }
        }
        else
        {
            iItem = -1;
        }
    }

    EckAssert(iItem >= -1);
    EckAssert(iItem <= SELRANGE_MAXVALUE);

    *piItem = iItem;
    return S_OK;
}

LONG CLVRange::InvertRange(LONG iBegin, LONG iEnd)
{
    sr.InvertRange(iBegin, iEnd);
    LONG iFirst;   // index before or contains iBegin value
    BOOL fSelect;  // are we selecting or unselecting
    LONG iTempE;
    LONG iTempB;
    HRESULT hres = S_OK;

    EckAssert(iEnd >= iBegin);
    EckAssert(iBegin >= SELRANGE_MINVALUE);
    EckAssert(iEnd <= SELRANGE_MAXVALUE);

    // find if first is selected or not
    fSelect = !_FindValue(iBegin, &iFirst);

    iTempE = iBegin - 1;

    do
    {
        iTempB = iTempE + 1;

        if (fSelect)
            NextSelected(iTempB, &iTempE);
        else
            NextUnSelected(iTempB, &iTempE);

        if (-1 == iTempE)
        {
            iTempE = SELRANGE_MAXVALUE;
        }
        else
        {
            iTempE--;
        }

        iTempE = std::min(iTempE, iEnd);

        if (fSelect)
        {
            if (FAILED(hres = IncludeRange(iTempB, iTempE)))
            {
                return(hres);
            }
        }
        else
        {
            if (FAILED(hres = ExcludeRange(iTempB, iTempE)))
            {
                return(hres);
            }
        }

        fSelect = !fSelect;
    } while (iTempE < iEnd);

    CheckRangeEqual(&sr, this, 9999);
    return(hres);
}











void CWndMain::UpdateDpi(int iDpi)
{
	const int iDpiOld = m_iDpi;
	UpdateDpiInit(iDpi);

	// TODO: 更新字体大小
	HFONT hFont = eck::ReCreateFontForDpiChanged(m_hFont, iDpi, iDpiOld);
	eck::SetFontForWndAndCtrl(HWnd, hFont, FALSE);
	std::swap(m_hFont, hFont);
	DeleteObject(hFont);

	UpdateFixedUISize();
}

void CWndMain::UpdateFixedUISize()
{
	// TODO: 更新固定大小的控件

}

void CWndMain::ClearRes()
{
	DeleteObject(m_hFont);
}

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCT* lpCreateStruct)
{
	using namespace eck::Literals;
	eck::GetThreadCtx()->UpdateDefColor();

	UpdateDpiInit(eck::GetDpi(hWnd));
	m_hFont = eck::EzFont(L"微软雅黑", 9);

	HANDLE hFind;
	WIN32_FIND_DATAW wfd;
	auto hil = ImageList_Create(90, 90, ILC_COLOR32 | ILC_ORIGINALSIZE, 20, 20);
	auto hilSmall = ImageList_Create(40, 40, ILC_COLOR32 | ILC_ORIGINALSIZE, 20, 20);
	auto path = LR"(D:\TestPic\)"_rs;
	hFind = FindFirstFileW((path + LR"(*.png)").Data(), &wfd);
	int i{};
	do
	{
		IWICBitmapDecoder* pd;
		eck::CreateWicBitmapDecoder(
			(path + wfd.cFileName).Data(), pd);
		m_vItem.emplace_back(wfd.cFileName, i, (i % 10) ? 1 : 0);

		IWICBitmap* pb;
		//eck::CreateWicBitmap(pb, pd, 90, 90, eck::DefWicPixelFormat, WICBitmapInterpolationModeHighQualityCubic);
		eck::CreateWicBitmap(pb, pd);
		//eck::SaveWicBitmap(eck::Format(LR"(D:\TestPic\%s.png)", wfd.cFileName).Data(), pb);
		const auto h = eck::CreateHICON(pb);
		ImageList_AddIcon(hil, h);

		eck::CreateWicBitmap(pb, pd, 40, 40);
		const auto hSmall = eck::CreateHICON(pb);
		ImageList_AddIcon(hilSmall, hSmall);
		/*if (i == 41)
			break;*/
		++i;
	} while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);

	{
		{
			m_LV.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER |
				LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_EDITLABELS, 0,
				0, 0, 10, 10, hWnd, 0);
			m_LytH1.Add(&m_LV, {}, eck::LF_FILL, 1);

			m_LVGroup.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_AUTOARRANGE |
				LVS_SHOWSELALWAYS | LVS_REPORT | LVS_EDITLABELS, 0,
				0, 0, 10, 10, hWnd, 0);
			m_LVGroup.EnableGroupView(TRUE);
			m_LVGroup.SetImageList(hil, LVSIL_NORMAL);
			m_LVGroup.SetImageList(hilSmall, LVSIL_SMALL);
			m_LVGroup.InsertColumn(L"Col. 1", -1, 210);
			m_LVGroup.InsertColumn(L"Col. 2", -1, 210);
			m_LVGroup.InsertColumn(L"Col. 3", -1, 210);
			m_LVGroup.InsertColumn(L"Col. 4", -1, 210);
			InitLvGroup();
			m_LytH1.Add(&m_LVGroup, {}, eck::LF_FILL, 1);
		}
		m_Lyt.Add(&m_LytH1, {}, eck::LF_FILL, 1);

		{
			m_LVOd.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS |
				LVS_OWNERDATA | LVS_REPORT|LVS_EDITLABELS, 0,
				0, 0, 10, 10, hWnd, 0);
			m_LytH2.Add(&m_LVOd, {}, eck::LF_FILL, 1);
			m_LVOd.SetImageList(hil, LVSIL_NORMAL);
			m_LVOd.SetImageList(hilSmall, LVSIL_SMALL);

			m_LVOd.InsertColumn(L"Col. 1", -1, 210, 0, 0);
			m_LVOd.InsertColumn(L"Col. 2", -1, 210, 0, 1);
			m_LVOd.InsertColumn(L"Col. 3", -1, 210, 0, 2);
			m_LVOd.InsertColumn(L"Col. 4", -1, 210, 0, 3);
			m_LVOd.LveGetHeader().SetImageList(hilSmall);
			InitLvOd();

			m_LVTile.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | 
				LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_EDITLABELS, 0,
				0, 0, 10, 10, hWnd, 0);
			m_LytH2.Add(&m_LVTile, {}, eck::LF_FILL, 1);
			m_LVTile.SetImageList(hil, LVSIL_NORMAL);
			m_LVTile.SetImageList(hilSmall, LVSIL_SMALL);
			m_LVTile.InsertColumn(L"Col. 1", -1, 210);
			m_LVTile.InsertColumn(L"Col. 2", -1, 210);
			m_LVTile.InsertColumn(L"Col. 3", -1, 210);
			m_LVTile.InsertColumn(L"Col. 4", -1, 210);
			InitLvTile();
		}
		m_Lyt.Add(&m_LytH2, {}, eck::LF_FILL, 1);
	}
	//m_LV.InsertColumn(L"Column 1.");
	//EckCounter(10, i)
		//m_LV.InsertItem(eck::Format(L"Item %d", i).Data());
	//m_LV.SetTextClr(eck::Colorref::BrightYellow);


	auto& hdr = m_LV.LveGetHeader();
	hdr.Style |= HDS_DRAGDROP;

	m_LV.InsertColumn(L"Col. 1", -1, 210);
	m_LV.InsertColumn(L"Col. 2", -1, 210);
	m_LV.InsertColumn(L"Col. 3", -1, 210);
	m_LV.InsertColumn(L"Col. 4", -1, 210);
	m_LV.SetView(LV_VIEW_DETAILS);
	m_LV.SetImageList(hil, LVSIL_NORMAL);
	m_LV.SetImageList(hilSmall, LVSIL_SMALL);
	m_LV.SetImageList(hilSmall, LVSIL_STATE);
	m_LV.SetLVExtendStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_HEADERDRAGDROP | LVS_EX_CHECKBOXES);
	LVITEMW li;
	li.mask = LVIF_IMAGE;
	EckCounter(41, i)
	{
		const int idx = m_LV.InsertItem((std::to_wstring(i) + L" 项目测试").c_str(), INT_MAX, 0, i);
		m_LV.SetItemText(idx, 1, L"测试子项111");
		m_LV.SetItemText(idx, 2, L"测试子项 二");
		if (i % 2)
		{
			li.iItem = idx;
			li.iSubItem = 2;
			li.iImage = 4;
			m_LV.SetItem(&li);
		}
		if (i % 10 == 0)
		{
			li.mask = LVIF_INDENT;
			li.iItem = idx;
			li.iSubItem = 0;
			li.iIndent = 1;
			m_LV.SetItem(&li);
			li.mask = LVIF_IMAGE;
		}
	}
	m_LV.LveGetHeader().Style |= (HDS_OVERFLOW);

	eck::LVE_ITEM_EXT ie;
	ie.uMask = eck::LVE_IM_COLOR_BK;
	ie.idxItem = 2;
	ie.idxSubItem = -1;
	ie.Clr.crBk = eck::Colorref::Aqua;
	m_LV.LveSetItem(ie);
	ie.uMask |= eck::LVE_IM_COLOR_TEXT;
	ie.idxSubItem = 2;
	ie.Clr.crBk = eck::Colorref::Green;
	ie.Clr.crText = eck::Colorref::Red;
	m_LV.LveSetItem(ie);
	ie.uMask = eck::LVE_IM_COLOR_TEXT;
	ie.idxItem = 6;
	ie.idxSubItem = -1;
	ie.Clr.crText = eck::Colorref::Red;
	m_LV.LveSetItem(ie);

	auto hfontbig = eck::EzFont(L"微软雅黑", 11);
	m_LV.LveSetHeaderHeight(75);
	hdr.HeSetMainTextFont(hfontbig);
	hdr.HeGetItemData(0).rsSubText = L"无大标题居左";

	hdr.HeGetItemData(1).rsMainText = L"文件名";
	hdr.HeGetItemData(1).rsSubText = L"Col. 1";

	hdr.HeGetItemData(2).rsMainText = L"大小";
	hdr.HeGetItemData(2).rsSubText = L"Col. 2";
	hdr.HeGetItemData(3).rsMainText = L"修改日期";
	hdr.HeGetItemData(3).rsSubText = L"Col. 3";
	//hdr.HeSetExtTextAlignV(eck::Align::Far);
	HDITEMW hdi{};
	hdi.mask = HDI_FORMAT;
	hdi.fmt = HDF_STRING | HDF_RIGHT;
	//hdr.SetItem(0, &hdi);
	hdr.SetItem(1, &hdi);
	hdr.SetItem(2, &hdi);
	hdi.fmt |= HDF_SPLITBUTTON;
	hdr.SetItem(3, &hdi);

	hdr.HeSetUseExtText(TRUE);

	eck::SetFontForWndAndCtrl(hWnd, m_hFont);
	m_LV.MakePretty();
	m_LVOd.MakePretty();
	m_LVGroup.MakePretty();
	m_LVTile.MakePretty();
	auto plv = m_LV.GetLvObject();
	plv->SetSelectionFlags(1, 1);
	plv->Release();
	return TRUE;
}

void CWndMain::InitLvOd()
{
	using namespace eck;
	m_LVOd.LveSetOwnerDataProc([](eck::LveOd uMsg, void* pData, void* pParam)->LRESULT
		{
			const auto pv = (std::vector<ITEM>*)pParam;
			switch (uMsg)
			{
			case eck::LveOd::GetDispInfo:
			{
				const auto p = (eck::LVE_ITEM_EXT*)pData;
				const auto& e = (*pv)[p->idxItem];
				if (p->uMask & LVE_IM_TEXT)
				{
					if (p->idxSubItem == 0)
					{
						p->pszText = e.rsText.Data();
						p->cchText = e.rsText.Size();
					}
					else if (const int pos = e.rsText.FindChar(L'_'); pos >= 0 && p->idxSubItem == 1)
					{
						wcsncpy_s((PWSTR)p->pszText, p->cchText, e.rsText.Data(), pos);
						//p->pszText = nullptr;
						p->cchText = pos;
					}
					else
					{
						p->pszText = nullptr;
						p->cchText = 0;
					}
				}
				if (p->uMask & LVE_IM_IMAGE)
					p->idxImage = e.idxImage;
				if (p->uMask & LVE_IM_LPARAM)
					p->lParam = 0;
				if (p->uMask & LVE_IM_COLOR_ALL)
				{
					p->Clr = {};
					if (e.rsText.IsStartOf(L"Nodoka"))
					{
						p->Clr.crText = eck::Colorref::Pink;
					}
					else if (e.rsText.IsStartOf(L"Hinata"))
					{
						p->Clr.crText = eck::Colorref::Yellow;
					}
					else if (e.rsText.IsStartOf(L"Chiyu"))
					{
						p->Clr.crText = eck::Colorref::CyanBlue;
					}
					else if (e.rsText.IsStartOf(L"Asumi"))
					{
						p->Clr.crText = eck::Colorref::Purple;
					}
					if (p->idxSubItem == 1)
						p->Clr.crBk = eck::Colorref::Gray;
					else if (e.rsText.Find(L"Ex") >= 0)
						p->Clr.crBk = (~p->Clr.crText) & 0x00FFFFFF;
				}
				if (p->uMask & LVE_IM_INDENT)
					p->iIndent = 0;
			}
			return 0;
			case eck::LveOd::SetDispInfo:
			{
				const auto p = (eck::LVE_ITEM_EXT*)pData;
				auto& e = (*pv)[p->idxItem];
				if (p->uMask & LVE_IM_TEXT)
				{
					if (p->idxSubItem == 0)
					{
						e.rsText.DupString(p->pszText, p->cchText);
					}
				}
			}
			return 0;
			}
			return 0;
		}, &m_vItem);
	const auto pLv = m_LVOd.GetLvObject();
    const auto pLvRange = new eck::CLVRange{};
    pLv->SetRangeObject(LVSR_SELECTION, pLvRange);
    //pLv->SetRangeObject(LVSR_SELECTION, LVRange_Create());
	pLv->Release();
	pLvRange->Release();

	m_LVOd.LveSetOwnerDataBufferSize(9999);
	m_LVOd.SetView(LV_VIEW_DETAILS);
	m_LVOd.LveGetHeader().Style |= (HDS_CHECKBOXES | HDS_FILTERBAR | HDS_DRAGDROP | HDS_FULLDRAG);
	IWICBitmap* pbmp;
	eck::CreateWicBitmap(pbmp, LR"(D:\TestPic\Hinata_Sparkle_Ex_1819062360449208745_1.jpg.png)",
		40, 40);
	auto hbm = eck::CreateHBITMAP(pbmp);
	HDITEMW hdi{};
	hdi.mask = HDI_FORMAT | HDI_BITMAP;
	hdi.fmt = HDF_STRING | HDF_IMAGE | HDF_BITMAP | HDF_CHECKBOX;
	auto& hdr = m_LVOd.LveGetHeader();
	hdr.SetItem(0, &hdi);
	hdi.hbm = hbm;
	hdr.SetItem(1, &hdi);
	hdi.hbm = 0;
	hdi.fmt |= HDF_SPLITBUTTON;
	hdr.SetItem(2, &hdi);
	hdi.fmt &= ~HDF_SPLITBUTTON;
	hdr.SetItem(3, &hdi);
	m_LVOd.SetItemCount((int)m_vItem.size());
	m_LVOd.LveSetHeaderHeight(90);
	m_LVOd.HeaderTextColor = eck::Colorref::BrightYellow;
	hdr.HeGetItemData(2).crBk = eck::Colorref::Red;
	hdr.HeGetItemData(3).crTextBk = eck::Colorref::Green;
	hdr.Items[2].TextP = L"测试";
}

void CWndMain::InitLvGroup()
{
	LVITEMW li;
	li.mask = LVIF_IMAGE;

	LVGROUP lg{ sizeof(LVGROUP) };
	lg.mask = LVGF_HEADER | LVGF_FOOTER | LVGF_SUBTITLE | LVGF_STATE | LVGF_TASK | LVGF_DESCRIPTIONTOP |
		LVGF_DESCRIPTIONBOTTOM | LVGF_SUBSETITEMS | LVGF_GROUPID;

	lg.pszFooter = (PWSTR)L"Footer测试";
	lg.pszSubtitle = (PWSTR)L"Subtitle测试";
	lg.pszTask = (PWSTR)L"Task测试";
	lg.pszDescriptionTop = (PWSTR)L"DescriptionTop测试";
	lg.pszDescriptionBottom = (PWSTR)L"DescriptionBottom测试";
	lg.state = lg.stateMask = LVGS_COLLAPSIBLE;

	lg.iGroupId = 1;
	lg.pszHeader = (PWSTR)L"Nodoka";
	m_LVGroup.InsertGroup(INT_MAX, &lg);

	lg.iGroupId = 2;
	lg.pszHeader = (PWSTR)L"Chiyu";
	m_LVGroup.InsertGroup(INT_MAX, &lg);

	lg.iGroupId = 3;
	lg.pszHeader = (PWSTR)L"Hinata";
	m_LVGroup.InsertGroup(INT_MAX, &lg);

	lg.iGroupId = 4;
	lg.pszHeader = (PWSTR)L"Asumi";
	m_LVGroup.InsertGroup(INT_MAX, &lg);

	lg.iGroupId = 5;
	lg.pszHeader = (PWSTR)L"Others";
	m_LVGroup.InsertGroup(INT_MAX, &lg);

	EckCounter(41, i)
	{
		const auto& e = m_vItem[i];
		if (e.rsText.IsStartOf(L"Nodoka"))
			lg.iGroupId = 1;
		else if (e.rsText.IsStartOf(L"Chiyu"))
			lg.iGroupId = 2;
		else if (e.rsText.IsStartOf(L"Hinata"))
			lg.iGroupId = 3;
		else if (e.rsText.IsStartOf(L"Asumi"))
			lg.iGroupId = 4;
		else
			lg.iGroupId = 5;
		li.mask = LVIF_IMAGE | LVIF_GROUPID | LVIF_TEXT;
		li.iItem = i;
		li.iSubItem = 0;
		li.pszText = (PWSTR)e.rsText.Data();
		li.iImage = e.idxImage;
		li.iGroupId = lg.iGroupId;
		const int idx = m_LVGroup.InsertItem(&li);
		m_LVGroup.SetItemText(idx, 1, L"测试子项111");
		m_LVGroup.SetItemText(idx, 2, L"测试子项 二");
		if (i % 2)
		{
			li.mask = LVIF_IMAGE;
			li.iItem = idx;
			li.iSubItem = 2;
			li.iImage = 4;
			m_LVGroup.SetItem(&li);
		}
		if (i % 10 == 0)
		{
			li.mask = LVIF_INDENT;
			li.iItem = idx;
			li.iSubItem = 0;
			li.iIndent = 1;
			m_LVGroup.SetItem(&li);
			li.mask = LVIF_IMAGE;
		}
	}
}

void CWndMain::InitLvTile()
{
	m_LVTile.SetView(LV_VIEW_TILE);
	LVTILEVIEWINFO tvi{ sizeof(LVTILEVIEWINFO) };
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;// | LVTVIM_LABELMARGIN;
	tvi.dwFlags = LVTVIF_FIXEDSIZE;
	tvi.sizeTile = { 300,100 };
	tvi.cLines = 2;
	tvi.rcLabelMargin = { 10,10,10,10 };
	m_LVTile.SetTileViewInfo(&tvi);
	UINT idxCol[]{ 1,2 };
	int fmtCol[]{ LVCFMT_LEFT,LVCFMT_LEFT };
	LVTILEINFO ti{ sizeof(LVTILEINFO) };
	ti.cColumns = 2;
	ti.piColFmt = fmtCol;
	ti.puColumns = idxCol;
	LVITEMW li;
	eck::LVE_ITEM_EXT ie;
	ie.uMask = eck::LVE_IM_COLOR_TEXT;
	ie.idxSubItem = 1;
	EckCounter(41, i)
	{
		const auto& e = m_vItem[i];

		li.mask = LVIF_IMAGE | LVIF_TEXT;
		li.iItem = i;
		li.iSubItem = 0;
		li.pszText = (PWSTR)e.rsText.Data();
		li.iImage = e.idxImage;
		const int idx = m_LVTile.InsertItem(&li);
		m_LVTile.SetItemText(idx, 1, L"测试子项111");
		m_LVTile.SetItemText(idx, 2, L"测试子项 二");
		ti.iItem = idx;
		m_LVTile.SetTileInfo(&ti);

		if (e.rsText.IsStartOf(L"Nodoka"))
			ie.Clr.crText = eck::Colorref::Pink;
		else if (e.rsText.IsStartOf(L"Chiyu"))
			ie.Clr.crText = eck::Colorref::CyanBlue;
		else if (e.rsText.IsStartOf(L"Hinata"))
			ie.Clr.crText = eck::Colorref::Yellow;
		else if (e.rsText.IsStartOf(L"Asumi"))
			ie.Clr.crText = eck::Colorref::Purple;
		else
			ie.Clr.crText = CLR_DEFAULT;
		if (ie.Clr.crText != CLR_DEFAULT)
		{
			ie.idxItem = idx;
			m_LVTile.LveSetItem(ie);
		}
	}
}

CWndMain::~CWndMain()
{

}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		m_Lyt.Arrange(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_NOTIFY:
	{
		const auto* const pnm = (NMHDR*)lParam;
			switch (pnm->code)
			{
			case LVN_ENDLABELEDITW:
				return TRUE;
			}
	}
	break;
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	case WM_COMMAND:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:

			return 0;
		}
	}
	break;
	case WM_DESTROY:
		ClearRes();
		PostQuitMessage(0);
		return 0;
	case WM_SETTINGCHANGE:
	{
		if (eck::IsColorSchemeChangeMessage(lParam))
		{
			const auto ptc = eck::GetThreadCtx();
			ptc->UpdateDefColor();
			ptc->SetNcDarkModeForAllTopWnd(ShouldAppsUseDarkMode());
			ptc->SendThemeChangedToAllTopWindow();
			return 0;
		}
	}
	break;
	case WM_DPICHANGED:
		eck::MsgOnDpiChanged(hWnd, lParam);
		UpdateDpi(HIWORD(wParam));
		return 0;
	}
	return CForm::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CWndMain::PreTranslateMessage(const MSG& Msg)
{
	return CForm::PreTranslateMessage(Msg);
}

HWND CWndMain::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData)
{
	IntCreate(dwExStyle, WCN_MAIN, pszText, dwStyle,
		x, y, cx, cy, hParent, hMenu, eck::g_hInstance, NULL);
	return NULL;
}