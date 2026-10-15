/**
 * AutosaveSquirrelWidget: an autosave status indicator starring a squirrel. It naps hugging an acorn when
 * everything is saved, gets alert when there are unsaved changes, hops back and forth stashing acorns into its
 * tree while saving, and facepalms after dropping the acorn when saving fails. It debounces autosave requests and
 * retries failed saves.
 */
#ifndef QFUNNYWIDGETS_AUTOSAVE_H
#define QFUNNYWIDGETS_AUTOSAVE_H

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QWidget>
#include <functional>

class QPainter;
class QTimer;

/**
 * Autosave status with a squirrel.
 */
class AutosaveSquirrelWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Status status READ getStatus WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(int autosaveDelay READ getAutosaveDelay WRITE setAutosaveDelay)
    Q_PROPERTY(int retryDelay READ getRetryDelay WRITE setRetryDelay)
    Q_PROPERTY(QColor squirrelColor READ getSquirrelColor WRITE setSquirrelColor)

public:
    enum Status
    {
        Saved = 0,
        Unsaved = 1,
        Saving = 2,
        Error = 3
    };
    Q_ENUM(Status)

    static QColor statusColor(int status);

    struct Options
    {
        int status = AutosaveSquirrelWidget::Saved;
        int autosaveDelay = 1500;  ///< ms of quiet before saveRequested (0 = never automatically)
        int retryDelay = 3000;     ///< ms before retrying a failed save (0 = never)
        QColor squirrelColor = QColor(QStringLiteral("#C1693C"));
        std::function<void()> saveRequested;
        std::function<void(int)> statusChanged;
    };

    explicit AutosaveSquirrelWidget(QWidget *parent = nullptr);
    explicit AutosaveSquirrelWidget(const Options &options, QWidget *parent = nullptr);

    Status getStatus() const;
    int getAutosaveDelay() const;
    int getRetryDelay() const;
    QColor getSquirrelColor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void markDirty();
    void requestSave();
    void saveFinished(bool ok);
    void setStatus(int status);
    void setAutosaveDelay(int autosaveDelay);
    void setRetryDelay(int retryDelay);
    void setSquirrelColor(const QColor &color);

signals:
    void saveRequested();
    void statusChanged(int status);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    qreal now() const;
    void changeStatus(int status);
    QString ago() const;
    void drawScene(QPainter &p, const QRectF &icon, qreal s, int st, qreal since, qreal now);
    void drawSquirrel(QPainter &p, const QPointF &b, qreal s, int st, qreal since, qreal now);
    static void drawAcorn(QPainter &p, const QPointF &c, qreal r, qreal rot);

    // Options.
    Status m_status;
    QColor m_squirrelColor;
    // Debounce / retry timers.
    QTimer *m_autosave;
    QTimer *m_retry;
    // State.
    qreal m_statusAt = 0.0;
    qreal m_savedAt = 0.0;
    bool m_dirtyAgain = false;
    qreal m_blinkAt = 2.0;
    // Timing.
    QElapsedTimer m_clock;
    QTimer *m_timer;
};

#endif // QFUNNYWIDGETS_AUTOSAVE_H
