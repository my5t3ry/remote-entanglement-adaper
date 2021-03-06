#ifndef WOLF_WIDGET_BAR_HPP_INCLUDED
#define WOLF_WIDGET_BAR_HPP_INCLUDED

#include "Widget.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class WidgetBar : public NanoWidget
{
  public:
    explicit WidgetBar(NanoWidget *widget, Size<uint> size) noexcept;

    void setFillPaint(Paint fillPaint);
    void setStrokePaint(Paint strokePaint);
    void setStrokeWidth(const float strokeWidth);
    
  protected:
    void onNanoDisplay() override;

  private:
    Paint fFillPaint;
    Paint fStrokePaint;
    float fStrokeWidth;

    DISTRHO_LEAK_DETECTOR(WidgetBar)
};

END_NAMESPACE_DISTRHO

#endif