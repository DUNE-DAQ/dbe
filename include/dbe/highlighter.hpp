#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QList>
#include <QMutex>
#include <QTextCharFormat>

class QTextDocument;
namespace daq {
namespace QTUtils {
/**
 * This class provides custom highlighting of textedits. It is possible to define any number of patterns (using regular
 * expressions) and a format to be applied to the given pattern. It also provides functionality to highlight all
 * occocurences of a given text using the
 * \code highlightAll(const QString &txt, bool caseSensitive) \endcode method.
 * For example in dvs_gui it is used to highlight the log outputs.
 * Usage:
 * \code
 * Highlighter* hl = new Highlighter(textedit->document());
 * QTextCharFormat format;
 * format.setForeGround(Qt::darkRed);
 * pattern = QRegExp("FAILED");
 * hl->setRule(pattern,format);
 * \endcode
 */
class Highlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  Highlighter(QTextDocument* parent = 0); ///< \brief ctor
  /**
   * Adds a highlighting rule to the list.
   * We need to clear the cache in this case (though normally rules are added very rarely)
   * \param r Regular expression to identify the text to highlight
   * \param f format to apply to the text
   */
  void setRule(QRegExp r, QTextCharFormat f)
  {
    HighlightingRule rule;
    rule.pattern = r;
    rule.format = f;
    highlightingRules.append(rule);
    cache.clear();
  }
  /**
   * Updates the text to highlight and rechecks the entire text
   * \param txt The text to highlight
   * \param caseSensitivity whether the highlighting is case sensitive or not
   */
  void highlightAll(const QString& txt, bool caseSensitivity)
  {
    highlightAllReg.setPattern(txt);
    if (caseSensitivity)
      highlightAllReg.setCaseSensitivity(Qt::CaseSensitive);
    else
      highlightAllReg.setCaseSensitivity(Qt::CaseInsensitive);
    rehighlight();
  }

protected:
  void highlightBlock(const QString& text);

private:
  /**
   * Stores highlighting information
   */
  struct HighlightingRule
  {
    QRegExp pattern;
    QTextCharFormat format;
  };
  /**
   * Used to cache result of rules
   * stores rule + indexes and lengths where the regexp matches
   */
  struct FormatCache
  {
    HighlightingRule rule;
    QVector<int> indexes;
    QVector<int> lengths;
  };
  QVector<HighlightingRule> highlightingRules; ///< highlighting rules
  QRegExp highlightAllReg;                     ///< regexp containing text to highlight
  QMutex* mutex;                               ///<  mutex to protect highlightBlock method
  QHash<uint, QList<FormatCache>>
    cache; ///< stores result of formatting for a given QString text (using uint qHash(QString)  method)
};

}
}
#endif
