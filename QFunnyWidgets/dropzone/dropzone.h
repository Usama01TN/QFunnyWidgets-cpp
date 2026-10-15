/**
 * DropZoneMonsterWidget: a file drop zone guarded by a hungry blob monster. It watches the cursor, opens its mouth
 * and drools when acceptable files are dragged over it, makes a disgusted face at unsupported files, chomps
 * dropped files (spitting paper bits) and burps. Click to browse.
 */
#ifndef QFUNNYWIDGETS_DROPZONE_H
#define QFUNNYWIDGETS_DROPZONE_H

#include <QColor>
#include <QElapsedTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QWidget>
#include <functional>

// Symbol export for the shared library build (empty for the static library, or when compiling the sources directly).
#ifndef QFUNNYWIDGETS_EXPORT
#  if defined(QFUNNYWIDGETS_BUILD_SHARED)
#    define QFUNNYWIDGETS_EXPORT Q_DECL_EXPORT
#  elif defined(QFUNNYWIDGETS_SHARED)
#    define QFUNNYWIDGETS_EXPORT Q_DECL_IMPORT
#  else
#    define QFUNNYWIDGETS_EXPORT
#  endif
#endif

class QMimeData;
class QPainter;
class QTimer;

/**
 * File drop zone with a monster.
 */
class QFUNNYWIDGETS_EXPORT DropZoneMonsterWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList acceptedExtensions READ getAcceptedExtensions WRITE setAcceptedExtensions)
    Q_PROPERTY(QString hint READ getHint WRITE setHint)
    Q_PROPERTY(QString subtitle READ getSubtitle WRITE setSubtitle)
    Q_PROPERTY(QColor monsterColor READ getMonsterColor WRITE setMonsterColor)
    Q_PROPERTY(QColor accentColor READ getAccentColor WRITE setAccentColor)
    Q_PROPERTY(bool clickToBrowse READ isClickToBrowse WRITE setClickToBrowse)

public:
    static constexpr qreal EAT_TIME = 1.25;
    static constexpr qreal BURP_TIME = 0.9;

    /** A bit of chewed paper. */
    struct Bit
    {
        qreal x = 0.0;
        qreal y = 0.0;
        qreal vx = 0.0;
        qreal vy = 0.0;
        qreal rot = 0.0;
        qreal vrot = 0.0;
        qreal life = 1.0;
    };

    struct Options
    {
        QStringList acceptedExtensions;  ///< Empty = everything is tasty.
        QString hint = QStringLiteral("Drop files here");
        QString subtitle = QStringLiteral("He's hungry. Or click to browse.");
        QColor monsterColor = QColor(QStringLiteral("#7BD389"));
        QColor accentColor = QColor(QStringLiteral("#5E60CE"));
        bool clickToBrowse = true;
        std::function<void(const QStringList &)> filesDropped;
        std::function<void(const QStringList &)> filesRejected;
    };

    explicit DropZoneMonsterWidget(QWidget *parent = nullptr);
    explicit DropZoneMonsterWidget(const Options &options, QWidget *parent = nullptr);

    int getEaten() const;
    QStringList getAcceptedExtensions() const;
    QString getHint() const;
    QString getSubtitle() const;
    QColor getMonsterColor() const;
    QColor getAccentColor() const;
    bool isClickToBrowse() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void feed(const QStringList &paths);
    void browse();
    void setAcceptedExtensions(const QStringList &exts);
    void setHint(const QString &hint);
    void setSubtitle(const QString &subtitle);
    void setMonsterColor(const QColor &color);
    void setAccentColor(const QColor &color);
    void setClickToBrowse(bool clickToBrowse);

signals:
    void filesDropped(const QStringList &paths);
    void filesRejected(const QStringList &paths);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    enum Mode { IDLE, HUNGRY, YUCK, EATING, REFUSED };

    qreal now() const;
    void setMode(Mode mode);
    bool accepts(const QString &path) const;
    void split(const QMimeData *mime, QStringList &good, QStringList &bad) const;
    void geometry(QRectF &zone, QPointF &c, qreal &s) const;
    void drawMonster(QPainter &p, qreal s, Mode mode, qreal since, qreal now);

    // Options.
    QStringList m_exts;
    QString m_hint;
    QString m_subtitle;
    QColor m_monsterColor;
    QColor m_accentColor;
    bool m_clickToBrowse;
    // State.
    Mode m_mode = IDLE;
    qreal m_modeAt = 0.0;
    int m_eaten = 0;
    int m_lastBatch = 0;
    QList<Bit> m_bits;
    qreal m_bitAcc = 0.0;
    qreal m_blinkAt = 2.0;
    qreal m_last = 0.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_DROPZONE_H
