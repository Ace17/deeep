#pragma once

#include <cstdio>

struct VideoCapture
{
  void captureDisplayFrameIfNeeded(Display* backend, Size2i dim)
  {
    if(m_captureFile || m_mustScreenshot)
    {
      vector<uint8_t> pixels(dim.width * dim.height * 4);
      backend->readPixels({ pixels.data(), (int)pixels.size() });

      if(m_captureFile)
        fwrite(pixels.data(), 1, pixels.size(), m_captureFile);

      if(m_mustScreenshot)
      {
        File::write("screenshot.rgba", pixels);
        fprintf(stderr, "Saved screenshot to 'screenshot.rgba'\n");

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
        fprintf(stderr, "Can't start video capture!\n");
        return false;
      }

      return true;
    }
    else
    {
      fprintf(stderr, "Stopped video capture\n");
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

