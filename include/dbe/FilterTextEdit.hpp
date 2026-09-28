#ifndef FILTERTEXTEDIT_H
#define FILTERTEXTEDIT_H
/**
 * \file FilterTextEdit.h
 * \author John.Erik.Sloper
 */
#include "ers/ers.hpp"
#include <QRegExp>
#include <QTextEdit>
namespace daq {
namespace QTUtils {

/**
 * Allows filtering of text in a text edit.
 * This will keep track of the 'original' text so that changes to the filter
 * will be redone on that text.
 * Example:
 * \code
 * FilterTextEdit fte;
 * fte.setPlainText("This is a first line\nThis is a second line");
 * \endcode
 *
 * It now shows:
 * \verbatim
This is a first line
This is a second line
\endverbatim
 * One can for example filter on the word 'first'
 * \code
 * fte.setFilter("first");
 * \endcode
 *
 * It now shows:
 * \verbatim
This is a first line
\endverbatim
 * If another filter is set the 'original' text is re-filtered:
 * \code
 * fte.setFilter("second");
 * \endcode
 * It now shows:
 * \verbatim
This is a second line
\endverbatim
 */
class FilterTextEdit : public QTextEdit
{
  Q_OBJECT
public:
  FilterTextEdit(QWidget* parent)
    : QTextEdit(parent)
    , m_invert(false)
  {
    ;
  }
  virtual ~FilterTextEdit() { ; }
  void append(const QString& text)
  {
    m_original.append(text);
    if (text.contains(m_filter)) {
      QTextEdit::append(text);
    }
  }
  void setPlainText(const QString& text)
  {
    m_original = text;
    filterAndSet(text);
  }

  void setFilter(const QString& filter)
  {
    m_filter = filter;
    this->filterAndSet(m_original);
  }
  void setInversion(bool invert) { m_invert = invert; }

private slots:
  void clear()
  {
    m_original = "";
    QTextEdit::clear();
  }

private:
  void filterAndSet(const QString& text)
  {
    if (m_filter != "") {
      QStringList strings = text.split("\n");
      if (!m_invert) {
        strings = strings.filter(m_filter);
      } else {
        QStringList result;
        foreach (QString str, strings) {
          if (!str.contains(m_filter)) {
            result += str;
          }
        }
        strings = result;
      }
      QString filtered = strings.join("\n");
      QTextEdit::setPlainText(filtered);
    } else {
      QTextEdit::setPlainText(text);
    }
  }

  QString m_filter;
  QString m_original; ///\brief the 'original' text.
  bool m_invert;      ///\brief inverts the filtering
};
}
}
#endif
