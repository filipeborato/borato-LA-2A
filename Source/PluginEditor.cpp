#include "PluginEditor.h"

BoratoLA2AAudioProcessorEditor::BoratoLA2AAudioProcessorEditor(BoratoLA2AAudioProcessor& processor)
    : AudioProcessorEditor(&processor), panel(processor)
{
    addAndMakeVisible(panel);
    setResizable(true, true);
    if (auto* resizeConstrainer = getConstrainer())
        resizeConstrainer->setFixedAspectRatio(1440.0 / 1080.0);
    setResizeLimits(720, 540, 1920, 1440);
    setSize(1080, 810);
#if BORATO_USE_OPENGL_RENDERER
    openGLContext.setContinuousRepainting(false);
    openGLContext.attachTo(*this);
#endif
}

BoratoLA2AAudioProcessorEditor::~BoratoLA2AAudioProcessorEditor()
{
#if BORATO_USE_OPENGL_RENDERER
    openGLContext.detach();
#endif
}

void BoratoLA2AAudioProcessorEditor::resized()
{
    panel.setBounds(getLocalBounds());
}
