#include "dbe/highlighter.hpp"
#include <QMutexLocker>
#include <QtGui>

// using namespace daq::QTUtils;
/**
 * Creates highlighter.
 * Initializes the mutex.
 */
Highlighter::Highlighter(QTextDocument* parent)
  : QSyntaxHighlighter(parent)
{
  mutex = new QMutex();
}

/**
 * Highlights a block of text according to the given rules (regexp + QTextCharFormat)
 * Uses a cache for performance reasons. This way we do not have to run regexps every time.
 * \param text text to highlight. Used to calculate the hash and hence cache value
 */
void
Highlighter::highlightBlock(const QString& text)
{
  QMutexLocker locker(mutex);

  /*
   * check if we have formatted this text before
   * If so there is no need to run regexp again and we can simply reapply the cached formatting.
   * Remember to clear the cache if new rules are added after it has been filled (not likely)
   */
  uint hash = qHash(text);
  if (cache.contains(hash)) {
    /**
     * Reapply the stored formattings
     */
    FormatCache form;
    foreach (form, cache.value(hash)) {
      for (int i = 0; i < form.indexes.size(); i++) {
        setFormat(form.indexes.at(i), form.lengths.at(i), form.rule.format);
      }
    }
  } else {
    /**
     * Apply formatting using regexp and store it in cache for later use
     */
    QList<FormatCache> forms; // list of all formatting being done
    foreach (HighlightingRule rule, highlightingRules) {
      FormatCache form;
      form.rule = rule; // store the rule
      QRegExp expression(rule.pattern);
      int index = text.indexOf(expression);
      int length;
      while (index >= 0) {

        length = expression.matchedLength();
        form.indexes.append(index);  // store index
        form.lengths.append(length); // store length
        setFormat(index, length, rule.format);
        index = text.indexOf(expression, index + length);
      }
      forms.append(form); // append this formatting (rule, indexes and lengths) to the list
    }
    cache.insert(hash, forms); // store list in cache
  }
  /**
   * Note. we should not cache highlighting as it changes all the time
   */
  if (!highlightAllReg.isEmpty()) {
    int index = text.indexOf(highlightAllReg);
    while (index >= 0) {
      int length = highlightAllReg.matchedLength();
      QTextCharFormat format;
      format.setBackground(Qt::yellow);
      setFormat(index, length, format);
      index = text.indexOf(highlightAllReg, index + length);
    }
  }
}
