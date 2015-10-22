#include <QScreen>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QGraphicsProxyWidget>
#include <QClipboard>
#include "cscreenshotview.h"
#include "cscreenshotscene.h"
#include "cscreenselectrectitem.h"
#include "cscreeneditortoolbaritem.h"
#include "cscreenrectitem.h"

#include <QUuid>
#include <QStandardPaths>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

CScreenShotView::CScreenShotView(QScreen *screen,
                                 QWidget *parent)
    :QGraphicsView(parent)
    ,m_desktopScreen(screen)
    ,m_screen(NULL)
    ,m_backgroundItem(NULL)
    ,m_selectRectItem(NULL)
    ,m_toolbarItem(NULL)
    ,m_currentRectItem(NULL)
    ,m_positionType(CSCREEN_POSITION_TYPE_NOT_CONTAIN)
    ,m_shotStatus(CSCREEN_SHOT_STATE_INITIALIZED)
    ,m_isPressed(false)
    ,m_isLocked(false)
{
   m_flags = this->windowFlags();
   this->setWindowModality(Qt::WindowModal);
    //======
//    this->overrideWindowFlags(Qt::ToolTip );
    m_screen = new CScreenShotScene(this);
    this->setScene(m_screen);
    QDesktopWidget * pDesktoWidget = QApplication::desktop();
    QRect geometry= screen->geometry();
    LOG_TEST(QString("screen->geometry() (%1,%2,%3,%4)")
             .arg(geometry.x())
             .arg(geometry.y())
             .arg(geometry.width())
             .arg(geometry.height()));

    QPixmap pixmap = screen->grabWindow(pDesktoWidget->winId(),geometry.x()
                                        ,geometry.y(),geometry.width(),geometry.height());

    drawPixmap(pixmap);
    //    m_pixmap = pixmap;
//    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
//            .append(QString("/%1.png").arg(QUuid::createUuid().toString()));
    //    pixmap.save(fileName);
//    qDebug()<<fileName<<geometry<<pixmap.size();
    m_backgroundItem = new QGraphicsPixmapItem(m_backgroundPixmap);
    m_screen->addItem(m_backgroundItem);
    this->setGeometry(geometry);
//    qDebug()<<"transform"<<this->transform();
    m_screen->setSceneRect(QRect(0,0,geometry.width(),geometry.height()));
    m_sx = 1.0 * geometry.width() / pixmap.width();
    m_sy = 1.0 * geometry.height() / pixmap.height();
    {//TYPE01
        //    qreal sy = 1.0 * geometry.height() / pixmap.height();
        //    this->scale(sx,sy);
        //    qDebug()<<"transform"<<this->transform();
        //    qreal dx = geometry.width() * (sx - 1) / 2 / sx;
        //    qreal dy = geometry.height() * (sy - 1) / 2 / sy;
        //    m_backgroundItem->moveBy(dx,dy);
    }
    {//TYPE02s
        m_backgroundItem->setScale(m_sx);
    }
    m_selectRectItem = new CScreenSelectRectItem(m_desktopPixmap);
    m_selectRectItem->setScale(m_sx);
    m_selectRectItem->setVisible(false);
    m_screen->addItem(m_selectRectItem);
    //====================
    m_toolbarItem = new CScreenEditorToolbarItem;
    connect(m_toolbarItem,SIGNAL(sigButtonClicked(CScreenButtonType)),
            this,SLOT(onButtonClicked(CScreenButtonType)));
    m_toolbarItem->setVisible(false);
    m_screen->addItem(m_toolbarItem);

}

CScreenShotView::~CScreenShotView()
{
    this->overrideWindowFlags(m_flags);
    if(m_screen)
    {
        m_screen->clear();
        delete m_screen;
        m_screen = NULL;
    }
}

void CScreenShotView::startSCreenShot()
{
    this->overrideWindowFlags(Qt::ToolTip);
    this->showFullScreen();
    this->overrideWindowFlags(Qt::Sheet | Qt::Window );
}

void CScreenShotView::setLocked(bool locked)
{
    if(m_isLocked == locked)
    {
        return;
    }
    m_isLocked = locked;
}

QPixmap CScreenShotView::getPixmap()
{
    return m_pixmap;
}

QPixmap CScreenShotView::createPixmap()
{
    QPixmap pixmap;
    if(m_shotStatus == CSCREEN_SHOT_STATE_SELECTED || m_shotStatus == CSCREEN_SHOT_STATE_EDITED)
    {
        QPointF startPos = /*getPointFromSelectedItem*/(m_selectRectItem->rect().topLeft());
        QPointF endPos = /*getPointFromSelectedItem*/(m_selectRectItem->rect().bottomRight());
        QRect rect = getPositiveRect(startPos,endPos);
        QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                .append(QString("/%1.png").arg(QUuid::createUuid().toString()));
//        QApplication::screens()
//        pixmap = QPixmap::grabWidget(this,rect);

        QDesktopWidget * pDesktoWidget = QApplication::desktop();
        QRect geometry= m_desktopScreen->geometry();
        LOG_TEST(QString("screen->geometry() (%1,%2,%3,%4)")
                 .arg(geometry.x())
                 .arg(geometry.y())
                 .arg(geometry.width())
                 .arg(geometry.height()));

        QPixmap desktopPixmap = m_desktopScreen->grabWindow(pDesktoWidget->winId(),geometry.x()
                                            ,geometry.y(),geometry.width(),geometry.height());

//        QPixmap desktopPixmap = m_desktopScreen->grabWindow(pDesktoWidget->winId());
        pixmap = desktopPixmap.copy(rect);
//        pixmap = desktopPixmap.opy(QRect(m_startPoint.x(),m_startPoint.y(),110,110));

        pixmap.save(fileName);
    }
    return pixmap;
}

bool CScreenShotView::event(QEvent *event)
{
    //    qDebug()<<"TTTTT"<<event->type();
    return QGraphicsView::event(event);
}

void CScreenShotView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        setShotStatus(CSCREEN_SHOT_STATE_CANCEL);
    }
    if(m_isLocked)
    {
        event->accept();
        return;
    }
}

void CScreenShotView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        setShotStatus(CSCREEN_SHOT_STATE_CANCEL);
        return;
    }
    if(m_isLocked)
    {
        event->accept();
        return;
    }
    if(event->button() == Qt::LeftButton)
    {
        LOG_TEST(QString("x %1,posX %2").arg(this->geometry().x()).arg(event->pos().x()));
        m_startPoint = event->pos();
        m_selectRect = m_selectRectItem->rect();
        bool isContains = m_selectRect.contains(getPointToSelectedItem(event->pos()));
        if((isContains
                && m_shotStatus == CSCREEN_SHOT_STATE_SELECTED)
                || m_shotStatus == CSCREEN_SHOT_STATE_INITIALIZED)
        {
            m_isPressed = true;
            m_toolbarItem->setVisible(false);
        }
        else if(m_shotStatus == CSCREEN_SHOT_STATE_EDITED
                && m_toolbarItem->getCurrentButtonType() == CSCREEN_BUTTON_TYPE_RECT
                && isContains
                && !m_isPressed)
        {
            m_isPressed = true;
            if(m_currentRectItem == NULL)
            {
                m_currentRectItem = createRectItem();
                m_currentRectItem->setVisible(false);
                m_screen->addItem(m_currentRectItem);
            }
        }

    }
    else
    {
        m_isPressed = false;
    }
    return QGraphicsView::mousePressEvent(event);
}

void CScreenShotView::mouseReleaseEvent(QMouseEvent *event)
{
    //
    if(m_isLocked)
    {
        event->accept();
        return;
    }

    if(m_isPressed)
    {
//        qDebug()<<"mouseReleaseEvent2";
        if(m_shotStatus == CSCREEN_SHOT_STATE_INITIALIZED)
        {
            setShotStatus(CSCREEN_SHOT_STATE_SELECTED);
        }
        m_selectRect = m_selectRectItem->rect();
        m_toolbarItem->setVisible(true);
        updateToolbarPosition();
    }

    //    m_selectRect = m_selectRectItem->rect();
    m_isPressed = false;

    QGraphicsView::mouseReleaseEvent(event);
//    m_screen->update();
}

void CScreenShotView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(m_isLocked)
    {
        event->accept();
        return;
    }
    //    QRect rect = getPositiveRect(m_startPoint,m_endPoint);
//    QRect rect = getPositiveRect(m_selectRectItem->rect().topLeft(),m_endPoint);
//    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
//            .append(QString("/%1.png").arg(QUuid::createUuid().toString()));
//    //    pixmap.save(fileName);
//    QPixmap::grabWidget(this,rect).save(fileName);
    return QGraphicsView::mouseDoubleClickEvent(event);
}

void CScreenShotView::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isLocked)
    {
        event->accept();
        return;
    }
    if((event->buttons() & Qt::LeftButton) && m_isPressed)
    {
        m_selectRectItem->setVisible(true);
        m_endPoint = event->pos();
        QPointF startPoint = getPointToSelectedItem(m_startPoint);
        QPointF endPoint = getPointToSelectedItem(event->pos());
        QPointF maxPoint = getPointToSelectedItem(QPointF(this->geometry().width(),this->geometry().height()));
        if(m_shotStatus == CSCREEN_SHOT_STATE_INITIALIZED)
        {
            QRect rect = getPositiveRect(startPoint,endPoint);
            m_selectRectItem->setSelectedRect(rect);
        }
        else if(m_shotStatus == CSCREEN_SHOT_STATE_SELECTED)
        {
            qreal dx = startPoint.x() - endPoint.x();
            qreal dy = startPoint.y() - endPoint.y();
            qreal x = -dx + m_selectRect.x();
            qreal y = -dy + m_selectRect.y();
            qreal maxX = maxPoint.x() - m_selectRect.width();
            qreal maxY = maxPoint.y() - m_selectRect.height();
            if(x < 0)
            {
                x = 0;
            }
            else if(x > maxX)
            {
                x = maxX;
            }

            if(y < 0)
            {
                y = 0;
            }
            else if(y >  maxY)
            {
                y = maxY;
            }
            QRectF rect(x,y,m_selectRect.width(),m_selectRect.height());
            m_selectRectItem->setSelectedRect(rect);
        }
        else if(m_shotStatus == CSCREEN_SHOT_STATE_EDITED && m_currentRectItem)
        {
            QRect rect = getPositiveRect(m_startPoint,event->pos());
            m_currentRectItem->setPainterRect(rect);
            m_currentRectItem->setVisible(true);
        }
    }
    return QGraphicsView::mouseMoveEvent(event);
}

CScreenRectItem *CScreenShotView::createRectItem()
{
    QPointF topLeftPos = getPointFromSelectedItem(m_selectRect.topLeft());
    QPointF bottomRightPos = getPointFromSelectedItem(m_selectRect.bottomRight());
    QRect rect = getPositiveRect(topLeftPos,bottomRightPos);

    CScreenRectItem *item = new CScreenRectItem(rect,QRectF(0,0,0,0));
//    QPen pen;
//    pen.setWidth(1);
//    pen.setColor(QColor(Qt::red));
//    item->setPen(pen);
//    item->setZValue(-1);
    return item;
}

void CScreenShotView::drawPixmap(const QPixmap &pixmap)
{
    m_desktopPixmap = pixmap;
    m_backgroundPixmap = pixmap;
    QPainter painter(&m_backgroundPixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(m_backgroundPixmap.rect(),QColor(0,0,0,90));
}

QPointF CScreenShotView::getPointToSelectedItem(const QPointF &point)
{
    return QPointF(point.x() / m_sx,point.y() / m_sy);
}

QPointF CScreenShotView::getPointFromSelectedItem(const QPointF &point)
{
    return QPointF(point.x() * m_sx,point.y() * m_sy);
}

QRect CScreenShotView::getPositiveRect(const QPointF &startPoint, const QPointF &endPoint)
{
    qreal width = endPoint.x() - startPoint.x();
    qreal height = endPoint.y() - startPoint.y();
    QPoint pos;
    if(width > 0)
    {
        pos.setX(startPoint.x());
    }
    else
    {
        pos.setX(endPoint.x());
    }
    if(height > 0)
    {
        pos.setY(startPoint.y());
    }
    else
    {
        pos.setY(endPoint.y());
    }
    QSize size(qAbs(width),qAbs(height));
    return QRect(pos,size);
}

void CScreenShotView::updateToolbarPosition()
{
    qreal x = m_selectRectItem->rect().left() * m_sx;
    qreal y = m_selectRectItem->rect().bottom() * m_sx + m_marginSelectedWidthToolbar;
    m_toolbarItem->setPos(x,y);
}

void CScreenShotView::setShotStatus(CScreenShotStatus status)
{
    if(m_shotStatus != status)
    {
        m_shotStatus = status;
        emit sigStatusChanged(m_shotStatus);
    }
}

void CScreenShotView::onButtonClicked(CScreenButtonType type)
{
    LOG_TEST(QString("onButtonClicked type %1").arg(type));
    switch (type)
    {
    case CSCREEN_BUTTON_TYPE_RECT:
        if(m_shotStatus == CSCREEN_SHOT_STATE_SELECTED)
        {
            setShotStatus(CSCREEN_SHOT_STATE_EDITED);
        }
        break;
    case CSCREEN_BUTTON_TYPE_OK:
        if(m_shotStatus == CSCREEN_SHOT_STATE_SELECTED || m_shotStatus == CSCREEN_SHOT_STATE_EDITED)
        {
            m_pixmap = createPixmap();
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setPixmap(m_pixmap);
        }
        LOG_TEST(QString("CSCREEN_SHOT_STATE_FINISHED type %1").arg(CSCREEN_SHOT_STATE_FINISHED));
        setShotStatus(CSCREEN_SHOT_STATE_FINISHED);
        break;
    case CSCREEN_BUTTON_TYPE_CANCLE:
        setShotStatus(CSCREEN_SHOT_STATE_CANCEL);
        break;
    default:
        break;
    }
}

