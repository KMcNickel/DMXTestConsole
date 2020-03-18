// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

/*******************************************************************************
  ExternalController Display Top-Level Driver Source File

  File Name:
    drv_gfx_external_controller.c

  Summary:
    Top level driver for external display controller.

  Description:
    Build-time generated implementation for the External Display Controller Driver.

    Created with MPLAB Harmony Version 3.0
*******************************************************************************/


#include "definitions.h"

#include "gfx/hal/inc/gfx_common.h"

#include "gfx/interface/drv_gfx_disp_intf.h"
#include "drv_gfx_external_controller.h"

#include "system/time/sys_time.h"

// Number of layers
#define LAYER_COUNT     1

#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  32

#define DISP_HOR_RESOLUTION DISPLAY_WIDTH
#define DISP_VER_RESOLUTION DISPLAY_HEIGHT

// Driver name
const char* DRIVER_NAME = "ExternalController";

#ifdef GFX_DISP_INTF_PIN_RESET_Clear
#define ExternalController_Reset_Assert()      GFX_DISP_INTF_PIN_RESET_Clear()
#define ExternalController_Reset_Deassert()    GFX_DISP_INTF_PIN_RESET_Set()
#else
#error "ERROR: GFX_DISP_INTF_PIN_RESET not defined. Please define in Pin Manager."
#define ExternalController_Reset_Assert()
#define ExternalController_Reset_Deassert()
#endif

#define ExternalController_NCSAssert(intf)   GFX_Disp_Intf_PinControl(intf, \
                                    GFX_DISP_INTF_PIN_CS, \
                                    GFX_DISP_INTF_PIN_CLEAR)

#define ExternalController_NCSDeassert(intf) GFX_Disp_Intf_PinControl(intf, \
                                    GFX_DISP_INTF_PIN_CS, \
                                    GFX_DISP_INTF_PIN_SET)

#define PIXEL_BUFFER_COLOR_MODE GFX_COLOR_MODE_RGB_565

/** SSD1963_STATE

  Summary:
    Enum of SSD1963 driver states.
    
*/
typedef enum
{
    INIT = 0,
    RUN
} ExternalController_STATE;

typedef struct ExternalController_drv 
{
    /* GFX context pointer */
    GFX_Context* gfx;
    
    /* Driver state */
    ExternalController_STATE state;
    
    /* Port-specific private data */
    void *port_priv;
} ExternalController_drv;

//command, parms
uint8_t cmd;
uint8_t parms[16];

/* ************************************************************************** */

/**
  Function:
    static void ExternalController_DelayMS(int ms)

  Summary:
    Delay helper function.

  Description:
    This is a helper function for delay using the system tick timer.

  Parameters:
    ms      - Delay in milliseconds

  Returns:
    None.

*/

static inline void ExternalController_DelayMS(int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;
    while (SYS_TIME_DelayIsComplete(timer) == false);
}

/**
  Function:
    static GFX_Result ExternalController_Reset(void)

  Summary:
    Toggles the hardware reset to the ExternalController.

  Description:
    This function toggles the GPIO pin for asserting reset to the ExternalController.

  Parameters:
    None

  Returns:
    None

*/
static GFX_Result ExternalController_Reset(void)
{
    ExternalController_Reset_Assert();
    ExternalController_DelayMS(10);
    ExternalController_Reset_Deassert();
    ExternalController_DelayMS(30);

    return GFX_SUCCESS;
}

static int ExternalController_Configure(ExternalController_drv *drv)
{
    GFX_Disp_Intf intf = (GFX_Disp_Intf) drv->port_priv;

    ExternalController_NCSAssert(intf);

    cmd = 0x0;
    GFX_Disp_Intf_WriteCommand(intf, cmd);

    cmd = 0x0;
    GFX_Disp_Intf_WriteCommand(intf, cmd);

    cmd = 0x0;
    GFX_Disp_Intf_WriteCommand(intf, cmd);

    ExternalController_NCSDeassert(intf);

    return 0;
}

static GFX_Result ExternalController_SetArea(
                                ExternalController_drv *drv,
                                int32_t x,
                                int32_t y,
                                uint32_t width,
                                uint32_t height)
{
    GFX_Disp_Intf intf;

    intf = (GFX_Disp_Intf) drv->port_priv;


    ExternalController_NCSAssert(intf);

    //Write X/Column Address
    parms[0] = x>>8;
    parms[1] = x;
    parms[2] = (x + width - 1) >>8;
    parms[3] = (x + width - 1);
    GFX_Disp_Intf_WriteCommand(intf, 0x0);
    GFX_Disp_Intf_WriteData(intf, parms, 4);
    
    //Write Y/Page Address
    parms[0] = y>>8;
    parms[1] = y;
    parms[2] = (y + height - 1)>>8;
    parms[3] = (y + height - 1);
    GFX_Disp_Intf_WriteCommand(intf, 0x0);
    GFX_Disp_Intf_WriteData(intf, parms, 4);

    return GFX_SUCCESS;
}

/**
  Function:
    static GFX_Color ExternalController_PixelGet(const GFX_PixelBuffer *buf,
                                      const GFX_Point *pnt)

  Summary:
    HAL interface function for reading pixel value from the ExternalController GRAM.

  Description:
    This function uses the interface-specific call to read pixel value from
    the ExternalController GRAM.

  Parameters:
    buf     - GFX_PixelBuffer pointer where pixel value will be stored
    pnt     - GFX_Point pointer describing pixel position

  Returns:

    GFX_UNSUPPORTED   Operation is not supported
    GFX_FAILURE       Operation failed
    GFX_SUCCESS       Operation successful


*/
static GFX_Color ExternalController_PixelGet(const GFX_PixelBuffer *buf,
                                  const GFX_Point *pnt)
{
    GFX_Context *context = GFX_ActiveContext();
    ExternalController_drv *drv;
    GFX_Disp_Intf intf;
    uint16_t pixel;

    if (!context)
        return GFX_FAILURE;

    drv = (ExternalController_drv *) context->driver_data;
    
    intf = (GFX_Disp_Intf) drv->port_priv;
    
        
    ExternalController_NCSAssert(intf);

    ExternalController_SetArea(drv, pnt->x, pnt->y, 1, 1);
    GFX_Disp_Intf_WriteCommand(intf, 0x0);
    GFX_Disp_Intf_ReadData16(intf, (uint16_t *) &pixel, 1);
    
    ExternalController_NCSDeassert(intf);


    return pixel;
}

/**
  Function:
    static GFX_Result ExternalController_SetPixel(const GFX_PixelBuffer *buf,
                                       const GFX_Point *pnt,
                                       GFX_Color color)

  Summary:
    HAL interface function for writing pixel value to the ExternalController GRAM.

  Description:
    This function uses the interface-specific call to write pixel value to the
    ExternalController GRAM.


  Parameters:
    buf     - GFX_PixelBuffer pointer
    pnt     - GFX_Point pointer describing pixel position
    color   - pixel color value (RGB565)

  Returns:
    * GFX_SUCCESS       - Operation successful
    * GFX_FAILURE       - Operation failed

*/
static GFX_Result ExternalController_PixelSet(const GFX_PixelBuffer *buf,
                            const GFX_Point *pnt,
                            GFX_Color color)
{
    GFX_Context *context = GFX_ActiveContext();
    ExternalController_drv *drv;
    GFX_Disp_Intf intf;

    if (!context)
        return GFX_FAILURE;

    drv = (ExternalController_drv *) context->driver_data;
    
    intf = (GFX_Disp_Intf) drv->port_priv;

    ExternalController_NCSAssert(intf);

    ExternalController_SetArea(drv,
                                  pnt->x,
                                  pnt->y,
                                  1,
                                  1);

    GFX_Disp_Intf_WriteCommand(intf, 0x0);

    GFX_Disp_Intf_WriteData16(intf, (uint16_t *) &color, 1);
    ExternalController_NCSDeassert(intf);

    return GFX_SUCCESS;
}

GFX_Result ExternalController_fillRect(const GFX_Rect* pRect,
                            const GFX_DrawState* pState)
{
    int16_t      x, y, left, right, top, bottom;
    GFX_Context* pContext = GFX_ActiveContext();
    GFX_Layer*   pLayer  = pContext->layer.active;
    GFX_Rect     clipRect;
    
    ExternalController_drv *drv;
    GFX_Disp_Intf intf;
    
    drv = (ExternalController_drv *) pContext->driver_data;
    intf = (GFX_Disp_Intf) drv->port_priv;

    // a basic fill is an optimal case for this driver
    // everything else should go through software pixel pipeline
    if(pContext->orientation != GFX_ORIENTATION_0 ||
       pContext->mirrored != GFX_FALSE)
        return cpuDrawRect_Fill(pRect, pState);

    // clip against the physical pLayer bounds
    if(GFX_PixelBufferClipRect(&pLayer->buffers[pLayer->buffer_write_idx].pb,
                               pRect,
                               &clipRect) == GFX_FAILURE)
    { // Input rectangle doesn't intersect with clip rectangle, we're done!
        return GFX_FAILURE;
    }

  #if GFX_BOUNDS_CLIPPING_ENABLED
    // clip against the global clipping rectangle
    if(pState->clipEnable == GFX_TRUE)
    {
        if(GFX_RectIntersects(pRect, &pState->clipRect) == GFX_FALSE)
            return GFX_SUCCESS;

        GFX_RectClip(pRect, &pState->clipRect, &clipRect);
    }
    else
    { // Clipping not on
        clipRect = *pRect;
    }
  #else
    clipRect = *pRect; // Clipping disabled.
  #endif


    ExternalController_NCSAssert(intf);

    ExternalController_SetArea(drv,
                              clipRect.x,
                              clipRect.y,
                              clipRect.width,
                              clipRect.height);

    GFX_Disp_Intf_WriteCommand(intf, 0x0);
    
    left   = clipRect.x;
    right  = left + clipRect.width;
    top    = clipRect.y;
    bottom = top + clipRect.height;
    
    for(y=top; y<bottom+1; y++)
    {
        for(x=left; x<right+1; x++)
        {
            GFX_Disp_Intf_WriteData16(intf, (uint16_t *) &pState->color, 1);
        }
    }

    ExternalController_NCSDeassert(intf);

    return(GFX_SUCCESS);
}

/**
  Function:
    static GFX_Result ExternalController_BrightnessRangeGet(uint32_t *low, uint32_t *high)

  Summary:
    Driver-specific implementation of GFX HAL brightnessRangeGet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
     * GFX_UNSUPPORTED      - Operation not supported

*/
static GFX_Result ExternalController_BrightnessRangeGet(uint32_t *low, uint32_t *high)
{
    *low = 0;
    *high = 100;
    
    return GFX_SUCCESS;
}

/**
  Function:
    static GFX_Result ExternalController_BrightnessSet(uint32_t val)

  Summary:
    Driver-specific implementation of GFX HAL brightnessSet function

  Description:
    Sets the ExternalController PWM Freq and Duty cycle

  Parameters:
    val    - The backlight brightness in %

  Returns:
    * GFX_SUCCESS       - Operation successful

*/
static GFX_Result ExternalController_BrightnessSet(uint32_t brightness)
{
    //Not supported
    return GFX_FAILURE;
}

/**
  Function:
    static GFX_Result ExternalController_Update(void)

  Summary:
    Driver-specific implementation of GFX HAL update function.

  Description:
    On GFX update, this function flushes any pending pixels to the ExternalController.

  Parameters:
    None.

  Returns:
    * GFX_SUCCESS       - Operation successful
    * GFX_FAILURE       - Operation failed

*/
static GFX_Result ExternalController_Update(void)
{
    GFX_Context *context = GFX_ActiveContext();
    GFX_Result returnValue = GFX_SUCCESS;
    ExternalController_drv *drv;

    if(context == NULL)
        return GFX_FAILURE;

    drv = (ExternalController_drv *) context->driver_data;

    if(drv->state == INIT)
    {
        // perform driver initialization here
        ExternalController_Reset();

        returnValue = ExternalController_Configure(drv);

        drv->state = RUN;
    }

    return returnValue;
}

/**
  Function:
    static void ExternalController_Destroy(GFX_Context *context)

  Summary:
    Driver-specific implementation of GFX HAL destroy function.

  Description:
    This function closes the ExternalController interface and frees up the data structures
    allocated by the driver.

  Parameters:
    context     - GFX context

  Returns:
    * GFX_SUCCESS       - Operation successful
    * GFX_FAILURE       - Operation failed

*/
static void ExternalController_Destroy(GFX_Context *context)
{
    // driver specific shutdown tasks
    ExternalController_drv *drv = NULL;
    GFX_Disp_Intf intf = 0;

    if (context != NULL && 
        context->driver_data != NULL)
    {
        drv = (ExternalController_drv *) context->driver_data;
        intf = (GFX_Disp_Intf) drv->port_priv;
        
        //Close the interface    
        ExternalController_NCSDeassert(intf);
        
        GFX_Disp_Intf_Close(intf);
        drv->port_priv = NULL;

        context->memory.free(context->driver_data);
        context->driver_data = GFX_NULL;

        // general default shutdown
        defDestroy(context);
    }
}

/**
  Function:
    static void ExternalController_Initialize(GFX_Context *context)

  Summary:
    Driver-specific implementation of GFX HAL initialize function.

  Description:
    This function creates driver-specific data structures, initializes data
    needed by HAL, and opens the port-specific interface to the ExternalController device.

  Parameters:
    context     - GFX context

  Returns:
    * GFX_SUCCESS       - Operation successful
    * GFX_FAILURE       - Operation failed

*/
static GFX_Result ExternalController_Initialize(GFX_Context *context)
{
    ExternalController_drv *drv;

    drv = (void *) context->memory.calloc(1, sizeof(ExternalController_drv));
    if (!drv)
        return GFX_FAILURE;

    drv->gfx = context;
    drv->state = INIT;

    context->layer.layers[0].enabled = GFX_TRUE;
    context->layer.layers[0].visible = GFX_TRUE;

    context->layer.layers[0].vsync = GFX_FALSE;
    context->layer.layers[0].swap = GFX_FALSE;

    context->layer.layers[0].rect.local.x = 0;
    context->layer.layers[0].rect.local.y = 0;
    context->layer.layers[0].rect.local.width = context->display_info->rect.width;
    context->layer.layers[0].rect.local.height = context->display_info->rect.height;

    context->layer.layers[0].rect.display = context->layer.layers[0].rect.local;

    context->layer.layers[0].alphaEnable = GFX_FALSE;
    context->layer.layers[0].alphaAmount = 255;

    context->layer.layers[0].maskEnable = GFX_FALSE;
    context->layer.layers[0].maskColor = 0;

    context->layer.layers[0].buffer_count = 1;
    context->layer.layers[0].buffer_read_idx = 0;
    context->layer.layers[0].buffer_write_idx = 0;

    GFX_PixelBufferCreate(DISPLAY_WIDTH,
                          DISPLAY_HEIGHT,
                          PIXEL_BUFFER_COLOR_MODE,
                          NULL,
                          &context->layer.layers[0].buffers[0].pb);

    context->layer.layers[0].buffers[0].state = GFX_BS_MANAGED;

    //Open interface to ExternalController controller
    drv->port_priv = (void *) GFX_Disp_Intf_Open(drv->gfx);
    if (drv->port_priv == 0)
    {
        if (drv)
            context->memory.free(drv);

        return GFX_FAILURE;
    }

    context->driver_data = (void *) drv;

    // general default initialization
    if(defInitialize(context) == GFX_FAILURE)
    {
        if (drv)
            context->memory.free(drv);

        return GFX_FAILURE;
    }

    return GFX_SUCCESS;
}

/**
  Function:
    static GFX_Result ExternalController_vsyncSet(GFX_Bool enable)

  Summary:
    Driver-specific implementation of GFX HAL layerVsyncSet function.

  Description:
     Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_vsyncSet(GFX_Bool enable)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_vsyncCallbackSet(GFX_SyncCallback_FnPtr cb)

  Summary:
    Driver-specific implementation of GFX HAL vsyncCallbackSet function.

  Description:
    Stub function, operation not supported in driver.


  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_vsyncCallbackSet(GFX_SyncCallback_FnPtr cb)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_hsyncCallbackSet(GFX_SyncCallback_FnPtr cb)

  Summary:
    Driver-specific implementation of GFX HAL hsyncCallbackSet function.

  Description:
    Stub function, operation not supported in driver.


  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_hsyncCallbackSet(GFX_SyncCallback_FnPtr cb)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    sstatic GFX_Result ExternalController_layerActiveSet(uint32_t idx)

  Summary:
    Driver-specific implementation of GFX HAL layerActiveSet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerActiveSet(uint32_t idx)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerEnabledSet(GFX_Bool val)

  Summary:
    Driver-specific implementation of GFX HAL layerEnabledSet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerEnabledSet(GFX_Bool val)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerPositionSet(int32_t x, int32_t y)

  Summary:
    Driver-specific implementation of GFX HAL layerPositionSet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerPositionSet(int32_t x, int32_t y)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerSizeSet(int32_t width, int32_t height)

  Summary:
    Driver-specific implementation of GFX HAL layerSizeSet function.

  Description:
     Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED     Operation not supported

*/
static GFX_Result ExternalController_layerSizeSet(int32_t width, int32_t height)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerBufferCountSet(uint32_t count)

  Summary:
    Driver-specific implementation of GFX HAL layerBufferCountSet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerBufferCountSet(uint32_t count)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerBufferAddressSet(uint32_t idx,
                                                    GFX_Buffer address)

  Summary:
    Driver-specific implementation of GFX HAL layerBufferAddressSet function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerBufferAddressSet(uint32_t idx, GFX_Buffer address)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerBufferCoherentSet(uint32_t idx,
                                                     GFX_Bool coherent)

  Summary:
    Driver-specific implementation of GFX HAL layerBufferCoherentSet function

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerBufferCoherentSet(uint32_t idx, GFX_Bool coherent)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerBufferAllocate(uint32_t idx)

  Summary:
    Driver-specific implementation of GFX HAL layerBufferAllocate function.

  Description:
    Stub function, operation not supported in driver.


  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerBufferAllocate(uint32_t idx)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerBufferFree(uint32_t idx)

  Summary:
    Driver-specific implementation of GFX HAL layerBufferFree function.

  Description:
    Stub function, operation not supported in driver.

  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerBufferFree(uint32_t idx)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerVisibleSet(GFX_Bool val)

  Summary:
    Driver-specific implementation of GFX HAL layerVisibleSet function.

  Description:
    Stub function, operation not supported in driver.


  Returns:
    * GFX_UNSUPPORTED       - Operation not supported

*/
static GFX_Result ExternalController_layerVisibleSet(GFX_Bool val)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    static GFX_Result ExternalController_layerAlphaEnableSet(GFX_Bool enable)

  Summary:
    Driver-specific implementation of GFX HAL layerAlphaEnableSet function.

  Description:
     Stub function, operation not supported in driver.


  Returns:

      GFX_UNSUPPORTED     Operation not supported

*/
static GFX_Result ExternalController_layerAlphaEnableSet(GFX_Bool enable, GFX_Bool wait)
{
    return GFX_UNSUPPORTED;
}

/**
  Function:
    GFX_Result driverExternalControllerContextInitialize(GFX_Context *context)

  Summary:
    Initializes the driver context.

  Description:
    Initializes the driver context.

  Returns:
    * GFX_SUCCESS       - Operation successful

*/
GFX_Result driverExternalControllerContextInitialize(GFX_Context *context)
{
    // set driver-specific data initialization function address
    context->hal.initialize = &ExternalController_Initialize;

    // override essential hal functions
    context->hal.destroy = &ExternalController_Destroy;
    context->hal.update = &ExternalController_Update;

    // set driver-specific function implementations
    context->hal.brightnessRangeGet = &ExternalController_BrightnessRangeGet;
    context->hal.brightnessSet = &ExternalController_BrightnessSet;
    context->hal.layerVsyncSet = &ExternalController_vsyncSet;
    context->hal.vsyncCallbackSet = &ExternalController_vsyncCallbackSet;
    context->hal.hsyncCallbackSet = &ExternalController_hsyncCallbackSet;
    context->hal.layerActiveSet = &ExternalController_layerActiveSet;
    context->hal.layerEnabledSet = &ExternalController_layerEnabledSet;
    context->hal.layerPositionSet = &ExternalController_layerPositionSet;
    context->hal.layerSizeSet = &ExternalController_layerSizeSet;
    context->hal.layerBufferCountSet = &ExternalController_layerBufferCountSet;
    context->hal.layerBufferAddressSet = &ExternalController_layerBufferAddressSet;
    context->hal.layerBufferCoherentSet = &ExternalController_layerBufferCoherentSet;
    context->hal.layerBufferAllocate = &ExternalController_layerBufferAllocate;
    context->hal.layerBufferFree = &ExternalController_layerBufferFree;
    context->hal.layerVisibleSet = &ExternalController_layerVisibleSet;
    context->hal.layerAlphaEnableSet = &ExternalController_layerAlphaEnableSet;

    context->hal.drawPipeline[GFX_PIPELINE_GCU].pixelSet = &ExternalController_PixelSet;
    context->hal.drawPipeline[GFX_PIPELINE_GCU].pixelGet = &ExternalController_PixelGet;

    context->hal.drawPipeline[GFX_PIPELINE_GCUGPU].pixelSet = &ExternalController_PixelSet;
    context->hal.drawPipeline[GFX_PIPELINE_GCUGPU].pixelGet = &ExternalController_PixelGet;
    
    context->hal.drawPipeline[GFX_PIPELINE_GCU].drawRect[GFX_DRAW_FILL][GFX_ANTIALIAS_OFF] = &ExternalController_fillRect;
    context->hal.drawPipeline[GFX_PIPELINE_GCUGPU].drawRect[GFX_DRAW_FILL][GFX_ANTIALIAS_OFF] = &ExternalController_fillRect;

    return GFX_SUCCESS;
}

/**
  Function:
    GFX_Result driverExternalControllerInfoGet(GFX_DriverInfo *info)

  Summary:
    HAL interface function for providing driver information.

  Description:
    This function provides driver information to the HAL.

  Parameters:
    info    - Pointer to driver information

  Returns:
    * GFX_SUCCESS       - Operation successful
    * GFX_FAILURE       - Operation failed

*/
GFX_Result driverExternalControllerInfoGet(GFX_DriverInfo *info)
{
    if(info == NULL)
        return GFX_FAILURE;

    // populate info struct
    strcpy(info->name, DRIVER_NAME);
    info->color_formats = GFX_COLOR_MASK_RGB_565;
    info->layer_count = LAYER_COUNT;

    return GFX_SUCCESS;
}

/**** End Hardware Abstraction Interfaces ****/
