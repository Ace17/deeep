// Game-specific : presentation

#include "base/audio.h"
#include "base/logger.h"
#include "base/renderer.h"

#include "presenter.h"

#include <string>

namespace
{
struct GamePresenter : IPresenter
{
  GamePresenter(IRenderer* renderer, Audio* audio) :
    m_renderer(renderer),
    m_audio(audio)
  {
  }

  ~GamePresenter()
  {
    if(m_musicVoice >= 0)
    {
      m_audio->stopVoice(m_musicVoice);
      m_audio->releaseVoice(m_musicVoice, true);
      m_musicVoice = -1;
    }
  }

  void flushFrame() override
  {
    if(m_textboxDelay > 0)
    {
      auto y = 2.0f;
      auto const DELAY = 90.0f;

      if(m_textboxDelay < DELAY)
        y += 16 * (DELAY - m_textboxDelay) / DELAY;

      RenderText text{};
      text.pos = Vec2f(0, y);
      text.text = m_textbox;
      m_renderer->drawText(text);
      m_textboxDelay--;
    }
  }

  void textBox(String msg) override
  {
    m_textbox.assign(msg.data, msg.len);
    m_textboxDelay = 60 * 2;
  }

  void preload(Resource res) override
  {
    switch(res.type)
    {
    case ResourceType::Sound:
      m_audio->loadSound(res.id, res.path);
      break;
    case ResourceType::Model:
      m_renderer->loadModel(res.id, res.path);
      break;
    }
  }

  void playMusic(MUSIC musicName) override
  {
    if(m_currMusicName == musicName)
      return;

    stopMusic();

    char buffer[256];
    m_audio->loadSound(1024, format(buffer, "res/music/music-%02d.ogg", musicName));

    m_musicVoice = m_audio->createVoice();
    logMsg("playing music #%d on voice %d", musicName, m_musicVoice);

    m_audio->playVoice(m_musicVoice, 1024, true);
    m_currMusicName = musicName;
  }

  void stopMusic() override
  {
    if(m_musicVoice == -1)
      return;

    m_audio->stopVoice(m_musicVoice); // maybe add a fade out here?
    m_audio->releaseVoice(m_musicVoice, true);
    m_musicVoice = -1;
    m_currMusicName = -1;
  }

  void playSound(SOUND soundId) override
  {
    auto voiceId = m_audio->createVoice();
    m_audio->playVoice(voiceId, soundId);
    m_audio->releaseVoice(voiceId, true);
  }

  void setCameraPos(Vec2f pos) override
  {
    m_renderer->setCamera(pos);
  }

  void setAmbientLight(float amount) override
  {
    m_renderer->setAmbientLight(amount);
  }

  void sendActor(SpriteActor const& actor) override
  {
    RenderSprite s;

    s.pos = actor.pos;
    s.halfSize = actor.scale;
    s.angle = actor.angle;
    s.useWorldRefFrame = !actor.screenRefFrame;
    s.modelId = (int)actor.model;
    s.blinking = actor.effect == Effect::Blinking;
    s.actionIdx = actor.action;
    s.frame = actor.ratio;
    s.zOrder = actor.zOrder;

    m_renderer->drawSprite(s);
  }

  void sendActor(DebugRectActor const& actor) override
  {
    Vec2f pos[4][2] =
    {
      { Vec2f(actor.pos[0].x, actor.pos[0].y), { Vec2f(actor.pos[0].x, actor.pos[1].y) } },
      { Vec2f(actor.pos[0].x, actor.pos[1].y), { Vec2f(actor.pos[1].x, actor.pos[1].y) } },
      { Vec2f(actor.pos[1].x, actor.pos[1].y), { Vec2f(actor.pos[1].x, actor.pos[0].y) } },
      { Vec2f(actor.pos[1].x, actor.pos[0].y), { Vec2f(actor.pos[0].x, actor.pos[0].y) } },
    };

    for(auto& line : pos)
    {
      RenderLine s {};
      s.useWorldRefFrame = true;
      s.a = line[0];
      s.b = line[1];
      s.color = { 8, 8, 0, 1 };
      s.thicknessMin = 0.5;
      s.thicknessMax = 0.1;
      s.zOrder = 100;
      m_renderer->drawLine(s);
    }
  }

private:
  IRenderer* const m_renderer;
  Audio* const m_audio;
  Audio::VoiceId m_musicVoice = -1;
  int m_currMusicName = -1;
  std::string m_textbox;
  int m_textboxDelay = 0;
};
}

IPresenter* createPresenter(IRenderer* renderer, Audio* audio)
{
  return new GamePresenter(renderer, audio);
}

