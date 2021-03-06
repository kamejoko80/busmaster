/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file      TXWindow.cpp
 * \author    Ratnadip Choudhury
 * \copyright Copyright (c) 2011, Robert Bosch Engineering and Business Solutions. All rights reserved.
 */
// TXWindow.cpp : Defines the initialization routines for the DLL.
//
#pragma once
#include "TxWindowFlexRay_stdafx.h"
#include <afxdllx.h>
//#include "../Application/MultiLanguage.h"
//#include "../Application/GettextBusmaster.h"
#define USAGE_EXPORT
#include "TxWndFlexRay_Extern.h"
#include "TxMsgChildFrame.h"
#include "LINScheduleDataStore.h"

CTxMsgChildFrame* g_pomTxMsgFlexChildWindow = nullptr;
CTxMsgChildFrame* g_pomTxMsgLinChildWindow = nullptr;
CScheduleTableCfgDlg* g_pomLINScheduleCfgDlg = NULL;

static AFX_EXTENSION_MODULE TXFlexRayWindowDLL = { false, nullptr };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    static HINSTANCE shLangInst=nullptr;


    // Remove this if you use lpReserved
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE0("TXWindowFlexRay.DLL Initializing!\n");

        // Extension DLL one-time initialization
        if (!AfxInitExtensionModule(TXFlexRayWindowDLL, hInstance))
        {
            return 0;
        }
        new CDynLinkLibrary(TXFlexRayWindowDLL);

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if (shLangInst)
        {
            FreeLibrary(shLangInst);
        }

        //Clear any memory allocated
        //CTxMsgManager::s_bDeleteTxMsgManager();
        TRACE0("TXWindowFlexRay.DLL Terminating!\n");

        // Terminate the library before destructors are called
        AfxTermExtensionModule(TXFlexRayWindowDLL);
    }
    return 1;   // ok
}


//Export Function Definitions.

void SetDefaultWindowPos(WINDOWPLACEMENT& sTxWndPlacement)
{
    sTxWndPlacement.flags = 1;
    sTxWndPlacement.length = 44;
    sTxWndPlacement.ptMaxPosition.x = 0;
    sTxWndPlacement.ptMaxPosition.x = 0;
    sTxWndPlacement.ptMinPosition.x = 0;
    sTxWndPlacement.ptMinPosition.y = 0;
    sTxWndPlacement.rcNormalPosition.top = 3;
    sTxWndPlacement.rcNormalPosition.bottom = 612;
    sTxWndPlacement.rcNormalPosition.left = 11;
    sTxWndPlacement.rcNormalPosition.right = 883;
    sTxWndPlacement.showCmd = 1;
}

HRESULT nShowScheduleConfigDlg(void* pParentWnd, ClusterConfig& pClusterConfig)
{
    HRESULT hResult = S_FALSE;

    if(g_pomLINScheduleCfgDlg == NULL)
    {
        g_pomLINScheduleCfgDlg = new CScheduleTableCfgDlg(pClusterConfig, (CWnd*)pParentWnd);

        hResult = g_pomLINScheduleCfgDlg->Create(IDD_DLG_LIN_SCHEDULE_CONFIG, (CWnd*)pParentWnd);
        //g_pomLINScheduleCfgDlg->SetParent((CWnd*)pParentWnd);
        WINDOWPLACEMENT wndPos;
        CLINScheduleDataStore::pGetLINSchedDataStore().vGetWindowPlacement(wndPos);
        g_pomLINScheduleCfgDlg->SetWindowPlacement(&(wndPos));
        g_pomLINScheduleCfgDlg->ShowWindow(SW_SHOW);
        hResult = S_OK;
    }
    else
    {
        g_pomLINScheduleCfgDlg->ShowWindow(SW_SHOW);
    }

    return hResult;
}
int nShowTxWindow( void* pParentWnd, ETYPE_BUS eBUS )
{
    CTxMsgChildFrame* pFrameWindow = nullptr;
    std::string strTitle = "";       //Used for Title.
    bool bValidBus = false;
    if ( eBUS == FLEXRAY )
    {
        pFrameWindow = g_pomTxMsgFlexChildWindow;
        strTitle = "Configure Transmission Messages - FLEXRAY";
        bValidBus = true;
    }
    else if ( eBUS == LIN )
    {
        pFrameWindow = g_pomTxMsgLinChildWindow;
        strTitle = "Configure Transmission Messages - LIN";
        bValidBus = true;
    }


    if ( bValidBus == false )
    {
        return S_FALSE;
    }


    if( pFrameWindow == nullptr )
    {
        pFrameWindow = new CTxMsgChildFrame(eBUS);
        if( pFrameWindow != nullptr )
        {
            //// Register Window Class
            LPCTSTR strMDIClass = AfxRegisterWndClass(
                                      CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                      LoadCursor(nullptr, IDC_CROSS), 0,
                                      ::LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICO_SEND_MSG)) );

            // Set the size got from configuration module
            WINDOWPLACEMENT sTxWndPlacement;
            SetDefaultWindowPos(sTxWndPlacement);
            WINDOWPLACEMENT* psTxWndPlacement = &sTxWndPlacement;
            bool bRetVal = false;
            if ( eBUS == FLEXRAY )
            {
                bRetVal = CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().bGetTxData(TX_WINDOW_PLACEMENT, (void**)&psTxWndPlacement);
            }
            else if ( eBUS == LIN )
            {
                bRetVal = (CTxLINDataStore::ouGetTxLINDataStoreObj().bGetTxData(TX_WINDOW_PLACEMENT, (void**)&psTxWndPlacement) == TRUE);
            }

            if ( bRetVal == false)//Load default configuration
            {
                CRect omRect;
                CWnd* pWnd = (CWnd*)pParentWnd;
                pWnd->GetClientRect(&omRect);
                omRect.NormalizeRect();
                //    // Reduce the size propotionally
                omRect.bottom -= (LONG)(omRect.Height() * defTX_MSG_WND_BOTTOM_MARGIN);
                omRect.right -= (LONG)(omRect.Width() * defTX_MSG_WND_RIGHT_MARGIN);
                //    // Update the size
                //sTxWndPlacement.rcNormalPosition = omRect;

                sTxWndPlacement.flags = 1;
                sTxWndPlacement.length = 44;
                sTxWndPlacement.ptMaxPosition.x = 0;
                sTxWndPlacement.ptMaxPosition.x = 0;
                sTxWndPlacement.ptMinPosition.x = 0;
                sTxWndPlacement.ptMinPosition.y = 0;
                sTxWndPlacement.rcNormalPosition.top = 1;
                sTxWndPlacement.rcNormalPosition.bottom = 661;
                sTxWndPlacement.rcNormalPosition.left = 4;
                sTxWndPlacement.rcNormalPosition.right = 864;
                sTxWndPlacement.showCmd = 1;
            }
            CRect omRect(&(sTxWndPlacement.rcNormalPosition));

            // Create Tx Message Configuration window
            //CRect omRect(0,0,200,200);
            if( pFrameWindow->Create( strMDIClass,
                                      strTitle.c_str(),
                                      WS_CHILD | WS_OVERLAPPEDWINDOW,
                                      omRect, (CMDIFrameWnd*)pParentWnd ) == TRUE )
            {
                // Show window and set focus
                pFrameWindow->ShowWindow( SW_SHOW);
                pFrameWindow->SetFocus();
                if (sTxWndPlacement.rcNormalPosition.top != -1 &&
                        sTxWndPlacement.length != 0)
                {
                    pFrameWindow->SetWindowPlacement(&sTxWndPlacement);
                }
                else
                {
                    CTxLINDataStore::ouGetTxLINDataStoreObj().bSetTxData(TX_WINDOW_PLACEMENT, (void**)&sTxWndPlacement);
                    // Set the defaul position
                    pFrameWindow->SetWindowPlacement(&sTxWndPlacement);
                }

            }
            if ( eBUS == FLEXRAY )
            {
                g_pomTxMsgFlexChildWindow = pFrameWindow;
            }
            else if ( eBUS == LIN )
            {
                g_pomTxMsgLinChildWindow = pFrameWindow;
            }
        }
        else
        {
            return S_FALSE;
        }
    }
    else
    {
        pFrameWindow->ShowWindow( SW_RESTORE );
        pFrameWindow->MDIActivate();
        pFrameWindow->SetActiveWindow();
    }
    return S_OK;
}



USAGEMODE HRESULT TXLIN_vShowScheduleConfigDlg(bool bShow)
{
    if ( NULL != g_pomLINScheduleCfgDlg )
    {
        int nShow  = bShow ? SW_SHOW : SW_HIDE;
        g_pomLINScheduleCfgDlg->ShowWindow(nShow);
    }
    return S_OK;
}


USAGEMODE HRESULT TXLIN_vCreateScheduleConfigDlg(void* pParentWnd, ClusterConfig& pClusterConfig)
{
    //Place this code at the beginning of the export function.
    //Save previous resource handle and switch to current one.
    HINSTANCE hInst = AfxGetResourceHandle();
    AfxSetResourceHandle(TXFlexRayWindowDLL.hResource);

    HRESULT nResult = nShowScheduleConfigDlg(pParentWnd, pClusterConfig);

    AfxSetResourceHandle(hInst);
    return nResult;
}

USAGEMODE HRESULT TXComman_vSetScheduleConfig( xmlDocPtr pDoc)
{

    CLINScheduleDataStore::pGetLINSchedDataStore().hSetConfigData(pDoc);
    if ( g_pomLINScheduleCfgDlg != NULL )
    {
        g_pomLINScheduleCfgDlg->nUpdateScheduleView();
    }

    return S_OK;
}

USAGEMODE HRESULT TXComman_vGetScheduleConfig( xmlNodePtr pDoc)
{
    WINDOWPLACEMENT wndPlacement;
    if ( g_pomLINScheduleCfgDlg != NULL )
    {
        g_pomLINScheduleCfgDlg->GetWindowPlacement(&wndPlacement);
    }
    else
    {
        CLINScheduleDataStore::pGetLINSchedDataStore().vGetWindowPlacement(wndPlacement);
    }
    //1. Save Window Positions
    xmlNodePtr pNodeWndPos = xmlNewNode(NULL, BAD_CAST DEF_WND_POS);
    xmlUtils::CreateXMLNodeFrmWindowsPlacement(pNodeWndPos,wndPlacement);
    xmlAddChild(pDoc, pNodeWndPos);
    CLINScheduleDataStore::pGetLINSchedDataStore().hGetConfigData(pDoc);
    return S_OK;
}
USAGEMODE HRESULT TXFlexRay_vShowConfigureMsgWindow(void* pParentWnd, ETYPE_BUS eBUS)
{
    //Place this code at the beginning of the export function.
    //Save previous resource handle and switch to current one.
    HINSTANCE hInst = AfxGetResourceHandle();
    AfxSetResourceHandle(TXFlexRayWindowDLL.hResource);

    int nRes = nShowTxWindow(pParentWnd, eBUS);

    AfxSetResourceHandle(hInst);
    return nRes;
}


USAGEMODE HRESULT TXComman_vSetClientID(ETYPE_BUS eBusType, DWORD dwClientID)
{
    if ( eBusType == FLEXRAY )
    {
    }
    else if ( eBusType == LIN )
    {
        CTxLINDataStore::ouGetTxLINDataStoreObj().m_dwClientID = dwClientID;
        CLINScheduleDataStore::pGetLINSchedDataStore().m_dwClientID = dwClientID;
    }
    return S_OK;
}
USAGEMODE HRESULT TXComman_vSetDILInterfacePtr(ETYPE_BUS eBusType, void* pDilPointer)
{
    if ( eBusType == FLEXRAY )
    {
        CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().bSetDILInterfacePtr();
    }
    else if ( eBusType == LIN )
    {
        CTxLINDataStore::ouGetTxLINDataStoreObj().bSetDILInterfacePtr((CBaseDIL_LIN*)pDilPointer);
    }

    return S_OK;
}

USAGEMODE HRESULT TXComman_vPostMessageToTxWnd(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ( wParam == 0 )  //eHEXDECCMD = 0;
    {
        CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().m_bHexMode = (bool)lParam;
        CTxLINDataStore::ouGetTxLINDataStoreObj().m_bHexMode = (bool)lParam;
    }


    if(g_pomTxMsgFlexChildWindow != nullptr)
    {
        g_pomTxMsgFlexChildWindow->m_pTransmitMsgView->SendMessage(msg, wParam, lParam);
    }
    if(g_pomTxMsgLinChildWindow != nullptr)
    {
        g_pomTxMsgLinChildWindow->m_pLinTransmitMsgView->SendMessage(msg, wParam, lParam);
    }

    CTxLINDataStore::ouGetTxLINDataStoreObj().nHandleKeyEvent((char)lParam);

    return S_OK;
}

USAGEMODE HRESULT TXComman_hConfigWindowShown(ETYPE_BUS eBusType)
{
    if ( eBusType == FLEXRAY )
    {
        if(g_pomTxMsgFlexChildWindow)
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
    }
    else if ( eBusType == LIN )
    {
        if(g_pomTxMsgLinChildWindow)
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
    }
    return S_OK;
}


USAGEMODE HRESULT TX_vBusStatusChanged(ETYPE_BUS eBusType, ESTATUS_BUS eBusStatus)
{
    if ( eBusType == FLEXRAY )
    {
        {
            CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().vSetBusStatus(eBusStatus);
        }
        if(g_pomTxMsgFlexChildWindow != nullptr)
        {
            g_pomTxMsgFlexChildWindow->m_pTransmitMsgView->SendMessage(WM_USER+44, 1, eBusStatus);
        }
    }
    else if ( eBusType == LIN )
    {
        CTxLINDataStore::ouGetTxLINDataStoreObj().vSetBusStatus(eBusStatus);
        CLINScheduleDataStore::pGetLINSchedDataStore().vSetBusStatus(eBusStatus);
    }

    //CTxMsgManager::s_podGetTxMsgManager()->vStopTransmission(unMaxWaitTime);
    return S_OK;
}

USAGEMODE HRESULT TXComman_vGetTxWndConfigData( ETYPE_BUS eBusType, xmlNodePtr pxmlNodePtr)
{
    if ( eBusType == FLEXRAY )
    {
        if(g_pomTxMsgFlexChildWindow)
        {
            g_pomTxMsgFlexChildWindow->vUpdateWndCo_Ords();
        }
        CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().bGetConfigData(pxmlNodePtr);
    }
    else
    {
        if(g_pomTxMsgLinChildWindow)
        {
            g_pomTxMsgLinChildWindow->vUpdateWndCo_Ords();
        }
        CTxLINDataStore::ouGetTxLINDataStoreObj().bGetConfigData(pxmlNodePtr);
    }

    return S_OK;
}


USAGEMODE HRESULT TXComman_vSetTxWndConfigDataXML( ETYPE_BUS eBusType, xmlDocPtr pDoc)
{
    if ( eBusType == FLEXRAY )
    {
        CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().bSetConfigData(pDoc);
    }
    else if ( eBusType == LIN )
    {
        CTxLINDataStore::ouGetTxLINDataStoreObj().bSetConfigData(pDoc);
    }
    return S_OK;
}

USAGEMODE HRESULT TXComman_nSetFibexConfig(ETYPE_BUS eBus, ClusterConfig& ouFlexConfig)
{
    HRESULT hr = S_FALSE;
    if ( eBus == FLEXRAY )
    {
        hr = CTxFlexRayDataStore::ouGetTxFlexRayDataStoreObj().SetFlexRayConfig(ouFlexConfig);
        if(g_pomTxMsgFlexChildWindow != nullptr && hr == S_OK)
        {
            g_pomTxMsgFlexChildWindow->m_pTransmitMsgView->UpdateTxView(ouFlexConfig.m_nChannelsConfigured);
        }
    }
    else if ( eBus == LIN )
    {
        hr = CTxLINDataStore::ouGetTxLINDataStoreObj().SetFlexRayConfig(&ouFlexConfig);
        if(g_pomTxMsgLinChildWindow != nullptr && hr == S_OK)
        {
            g_pomTxMsgLinChildWindow->m_pLinTransmitMsgView->UpdateTxView(ouFlexConfig.m_nChannelsConfigured);
        }
        CLINScheduleDataStore::pGetLINSchedDataStore().hSetClusterConfig(&ouFlexConfig);
        if (NULL != g_pomLINScheduleCfgDlg )
        {
            g_pomLINScheduleCfgDlg->nUpdateScheduleView();
        }

    }

    return hr;
}
