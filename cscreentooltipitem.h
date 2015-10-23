#ifndef CSCREENTOOLTIPITEM_H
#define CSCREENTOOLTIPITEM_H

#include <QGraphicsObject>
#include "clogsetting.h"

class CScreenTooltipItem : public QGraphicsObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

public:
    CScreenTooltipItem(QGraphicsItem *parent = 0);
    ~CScreenTooltipItem();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setText(const QString &text);

private:
    QRectF m_rect;
    QString m_text;
    QColor m_colorBackgroud;
    QColor m_colorText;
    static const int m_textPointSize = 12;
    static const qreal m_rectRadius = 2.5;
};

#endif // CSCREENTOOLTIPITEM_H
