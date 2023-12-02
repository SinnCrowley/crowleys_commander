#ifndef STYLETWEAKS_H
#define STYLETWEAKS_H

#import <QProxyStyle>

class StyleTweaks : public QProxyStyle {
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                           QPainter *painter, const QWidget *widget) const;
};

#endif // STYLETWEAKS_H
