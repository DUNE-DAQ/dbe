#ifndef QDEBUGSTREAM_H_
#define QDEBUGSTREAM_H_

#include "ers/ers.hpp"
#include <QCoreApplication>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QTextEdit>
#include <iostream>
#include <streambuf>
#include <string>
namespace daq {
namespace QTUtils {

/**
 * \class QDebugStream
 * Redirects a std::ostream (such as cout or cerr) to a text edit.
 * In order to reset the stream simply delete the QDebugStream object.\n
 * Usage:
 * \code
 * qout = new QDebugStream(std::cout, myTextEdit); //everything written to std::cout is appended to the text edit
 * ...
 * delete qout; //resets the streamoutput
 * \endcode
 */
class QDebugStream
  : public QObject
  , public std::basic_streambuf<char>
{
  Q_OBJECT
public:
  /**
   */
  QDebugStream(std::ostream& stream, QTextEdit* text_edit)
    : m_stream(stream)
  {
    m_mutex = new QMutex();
    log_window = text_edit;
    m_old_buf = stream.rdbuf();
    stream.rdbuf(this);
  }
  ~QDebugStream()
  {
    // output anything that is left
    if (!m_string.empty())
      log_window->append(m_string.c_str());

    m_stream.rdbuf(m_old_buf);
    delete m_mutex;
  }

protected:
  virtual int_type overflow(int_type v);
  virtual std::streamsize xsputn(const char* p, std::streamsize n);
  virtual bool event(QEvent* event);

private:
  std::ostream& m_stream;
  std::streambuf* m_old_buf;
  std::string m_string;
  QTextEdit* log_window;
  QMutex* m_mutex;
  QString m_text;
};
}
}
#endif /*QTEXTEDIT_H_*/
