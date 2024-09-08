#pragma once

#include "base/logger.h"

struct VideoCapture
{
  void captureDisplayFrameIfNeeded(IGraphicsBackend* backend, Vec2i dim)
  {
    if(m_captureFile || m_mustScreenshot)
    {
      std::vector<uint8_t> pixels(dim.x * dim.y * 4);
      backend->readPixels({ pixels.data(), (int)pixels.size() });

      if(m_captureFile)
        fwrite(pixels.data(), 1, pixels.size(), m_captureFile);

      if(m_mustScreenshot)
      {
        File::write("screenshot.rgba", pixels);
        logMsg("Saved screenshot to 'screenshot.rgba'");

        m_mustScreenshot = false;
      }
    }
  }

  bool toggleVideoCapture()
  {
    if(!m_captureFile)
    {
      m_captureFile = fopen("capture.rgba", "wb");

      if(!m_captureFile)
      {
        logMsg("Can't open capture.rgba for writing.");
        return false;
      }

      return true;
    }
    else
    {
      logMsg("Stopped video capture.");
      fclose(m_captureFile);
      m_captureFile = nullptr;
      return false;
    }
  }

  void takeScreenshot()
  {
    m_mustScreenshot = true;
  }

private:
  FILE* m_captureFile = nullptr;
  bool m_mustScreenshot = false;
};

