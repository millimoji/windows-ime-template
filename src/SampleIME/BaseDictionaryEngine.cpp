// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "BaseDictionaryEngine.h"
#include "SampleIMEGlobals.h"

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CBaseDictionaryEngine::CBaseDictionaryEngine(LCID locale, _In_ CFile *pDictionaryFile)
{
    _locale = locale;
    _pDictionaryFile = pDictionaryFile;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CBaseDictionaryEngine::~CBaseDictionaryEngine()
{
}

//+---------------------------------------------------------------------------
// SortListItemByFindKeyCode
//----------------------------------------------------------------------------

VOID CBaseDictionaryEngine::SortListItemByFindKeyCode(_Inout_ std::vector<CCandidateListItem> *pItemList)
{
    MergeSortByFindKeyCode(pItemList, 0, static_cast<int>(pItemList->size() - 1));
}

//+---------------------------------------------------------------------------
// MergeSortByFindKeyCode
//
//    Mergesort the array of element in CCandidateListItem::_FindKeyCode
//
//----------------------------------------------------------------------------

VOID CBaseDictionaryEngine::MergeSortByFindKeyCode(_Inout_ std::vector<CCandidateListItem> *pItemList, int leftRange, int rightRange)
{
    int candidateCount = CalculateCandidateCount(leftRange, rightRange);

    if (candidateCount > 2)
    {
        int mid = leftRange + (candidateCount / 2);

        MergeSortByFindKeyCode(pItemList, leftRange, mid);
        MergeSortByFindKeyCode(pItemList, mid, rightRange);

        std::vector<CCandidateListItem> ListItemTemp;

        int leftRangeTemp = 0;
        int midTemp = 0;
        for (leftRangeTemp = leftRange, midTemp = mid; leftRangeTemp != mid || midTemp != rightRange;)
        {
            CStringRange* psrgLeftTemp = nullptr;
            CStringRange* psrgMidTemp = nullptr;

            psrgLeftTemp = &pItemList->at(leftRangeTemp)._FindKeyCode;
            psrgMidTemp = &pItemList->at(midTemp)._FindKeyCode;

            ListItemTemp.push_back(CCandidateListItem());
            CCandidateListItem* pLI = &ListItemTemp.back();
            if (pLI)
            {
                if (leftRangeTemp == mid)
                {
                    *pLI = pItemList->at(midTemp++);
                }
                else if (midTemp == rightRange || CStringRange::Compare(_locale, psrgLeftTemp, psrgMidTemp) != CSTR_GREATER_THAN)
                {
                    *pLI = pItemList->at(leftRangeTemp++);
                }
                else
                {
                    *pLI = pItemList->at(midTemp++);
                }
            }
        }

        leftRangeTemp = leftRange;
        for (UINT count = 0; count < ListItemTemp.size(); count++)
        {
            pItemList->at(leftRangeTemp++) = ListItemTemp.at(count);
        }
    }
    else if (candidateCount == 2)
    {
        CStringRange *psrgLeft = nullptr;
        CStringRange *psrgLeftNext = nullptr;

        psrgLeft = &pItemList->at(leftRange)._FindKeyCode;
        psrgLeftNext = &pItemList->at(static_cast<size_t>(leftRange) + 1)._FindKeyCode;

        if (CStringRange::Compare(_locale, psrgLeft, psrgLeftNext) == CSTR_GREATER_THAN)
        {
            CCandidateListItem ListItem;
            ListItem = pItemList->at(leftRange);
            pItemList->at(leftRange ) = pItemList->at(static_cast<size_t>(leftRange) + 1);
            pItemList->at(static_cast<size_t>(leftRange) + 1) = ListItem;
        }
    }
}

int CBaseDictionaryEngine::CalculateCandidateCount(int leftRange,  int rightRange)
{
    assert(leftRange >= 0);
    assert(rightRange >= 0);

    return (rightRange - leftRange + 1);
}