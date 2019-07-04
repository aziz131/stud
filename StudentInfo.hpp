
#ifndef STUDENT_INFO_HPP_
#define STUDENT_INFO_HPP_

#include <Wt/WApplication.h>
#include <Wt/WMessageBox.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WIntValidator.h>

#include <string>
#include <array>
#include <algorithm>

namespace dbo = Wt::Dbo;

using namespace Wt;


 class Student {
 public:
     std::string name;
     int        born;

     template<class Action>
     void persist(Action& a)
     {
         dbo::field(a, name,     "name");
         dbo::field(a, born,     "born");
     }
 };

 class lesson {
 public:
     std::string name;
     int        test;

     template<class Action>
     void persist(Action& a)
     {
         dbo::field(a, name,     "name");
         dbo::field(a, test,     "test");

     }
 };

 class exam {
 public:
     std::string name_s;
     std::string name_l;
     int        mark;
     template<class Action>
     void persist(Action& a)
     {
         dbo::field(a, name_s,     "name_s");
         dbo::field(a, name_l,     "name_l");
         dbo::field(a, mark,     "mark");

     }
 };


class StudentInfo : public WApplication
{
public:

  StudentInfo(const WEnvironment& env ,Wt::Dbo::SqlConnectionPool* StudentDb);

private:
  std::unique_ptr<Dbo::backend::Sqlite3> sqlite;
  void setStudent();
  void setClass();
  void setMarks();
  void setMark(WString name,WString les,int t);
  void addstudent(WString n, WString b);
  void addlesson(WString n, WString t);
  void print(void);
  void printlesson(void);
  void printAllExam(void);
  void setStatus(const WString& text);
  int checkSL(WString name, WString le);
  void addexam(WString name, WString le, int m);
  Wt::Dbo::Session session;
  uint count;
  WText *status_;
};

//...............................
StudentInfo::StudentInfo(const WEnvironment& env ,Wt::Dbo::SqlConnectionPool* StudentDb)
  : Wt::WApplication(env)
{



  session.setConnectionPool(*StudentDb);
  session.mapClass<Student>("Student");
  session.mapClass<lesson>("lesson");
  session.mapClass<exam>("exam");


  try{
  Dbo::Transaction t(session);
  session.createTables();
  t.commit();
}catch (std::exception& e){
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
}


  setTitle("دانشجو");

  WContainerWidget *textdiv = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  textdiv->addWidget(cpp14::make_unique<WText>("<h2>ثبت نام دانشجو</h2>"));

  WContainerWidget *buttons = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  buttons->setStyleClass("buttons");

  WPushButton *button = buttons->addWidget(cpp14::make_unique<WPushButton>("ثبت نام دانشجو"));
  button->clicked().connect(this, &StudentInfo::setStudent);


  button = buttons->addWidget(cpp14::make_unique<WPushButton>("ثبت کلاس"));
  button->clicked().connect(this, &StudentInfo::setClass);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("نمره دهی"));
  button->clicked().connect(this, &StudentInfo::setMarks);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("لیست دروس"));
  button->clicked().connect(this, &StudentInfo::printlesson);
  button = buttons->addWidget(cpp14::make_unique<WPushButton>("لیست دانشجوها"));
  button->clicked().connect(this, &StudentInfo::print);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("امتحانات چاپ"));
  button->clicked().connect(this, &StudentInfo::printAllExam);

  textdiv = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  status_ = textdiv->addWidget(cpp14::make_unique<WText>("..."));

  styleSheet().addRule(".buttons",
		       "padding: 5px;");
  styleSheet().addRule(".buttons BUTTON",
		       "padding-left: 4px; padding-right: 4px;"
		       "margin-top: 4px; display: block");

  // avoid scrollbar problems
  styleSheet().addRule(".text", "padding: 4px 8px");
  styleSheet().addRule("body", "margin: 0px;");
}
//ثبت نام در کلاس و نمره
void StudentInfo::setMarks()
{
  WDialog dialog(" نمره دهی ");

  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);
  dialog.contents()->addStyleClass("form-group");
  WLineEdit *edit = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit->setPlaceholderText("دانشجو");
  WLineEdit *edit1 = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit1->setPlaceholderText("درس");

  WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("ثبت"));
  ok->setDefault(true);
  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("لغو"));
  ok1->setDefault(true);
  edit->setFocus();
  edit1->setFocus();
  dbo::Transaction tr (session);

  dbo::ptr<Student> joe = session.find<Student>().where("name = ?").bind("Joe");

  tr.commit();
  ok->clicked().connect([&]{int n =StudentInfo::checkSL(edit->text(), edit1->text());
                            if(n > 0){ StudentInfo::setMark(edit->text(), edit1->text(),n);
                                      dialog.accept();
                            } else { dialog.reject();
                            }  });
  ok1->clicked().connect(&dialog, &WDialog::reject);
  if (dialog.exec() == DialogCode::Accepted) {
    //  setStatus(edit->text());
  } else {
    setStatus("نمردهی انجام نشد");
  }
}
////////
int StudentInfo::checkSL(WString name, WString le){
  bool n,l;
  int t;
  dbo::Transaction tr (session);
  dbo::ptr<Student> stu = session.find<Student>().where("name = ?").bind(name.toUTF8());
  if (stu) {
    n = true;
  } else {
    n = false;
  }
  dbo::ptr<lesson> les = session.find<lesson>().where("name = ?").bind(le.toUTF8());
  if (les) {
    l = true;
    t = les->test;
  } else {
    l = false;
  }
  tr.commit();

  if (n&&l) {
    return t;
  } else {
    return 0;
  }
}

//...............
void StudentInfo::setMark(WString name, WString le, int u){
  WDialog dialog(" نمره دهی ");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);
  dialog.footer()->addWidget(cpp14::make_unique<WText>(name + " "+le));


    WLineEdit *edit3 = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
    edit3->setPlaceholderText("میان ترم");


  WLineEdit *edit2 = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit2->setPlaceholderText("پایان ترم");
  WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("ثبت"));
  ok->setDefault(true);
  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("لغو"));
  ok1->setDefault(true);
  edit2->setFocus();
  edit3->setFocus();
  WString t1=edit2->text();
  WString t2=edit3->text();
  ok->clicked().connect([&]{int av =(std::stol(t1.toUTF8())+std::stol(t2.toUTF8()))/2;StudentInfo::addexam(name,le,av);});
  ok1->clicked().connect(&dialog, &WDialog::reject);
  if (dialog.exec() == DialogCode::Accepted) {
      setStatus("" );
  } else {
    setStatus("نمردهی انجام نشد");
  }
}
void StudentInfo::addexam(WString name, WString le, int m){
  Dbo::Transaction t(session);
  std::unique_ptr<exam> stu{new exam};
  stu->name_s = name.toUTF8();
  stu->name_s = le.toUTF8();
  stu->mark = m;
  session.add(std::move(stu));
  t.commit();
}
///
//ثبت کلاس
////
void StudentInfo::setClass()
{
  WDialog dialog(" ثبت کلاس ");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);
  WLineEdit *edit = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit->setPlaceholderText("درس");
  WLineEdit *edit2 = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit2->setPlaceholderText("تعداد میان ترم");
  WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("ثبت"));
  ok->setDefault(true);
  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("لغو"));
  ok1->setDefault(true);
  edit->setFocus();
  edit2->setFocus();
  ok->clicked().connect([&]{StudentInfo::addlesson(edit->text(),edit2->text());dialog.accept();});
  ok1->clicked().connect(&dialog, &WDialog::reject);
  if (dialog.exec() == DialogCode::Accepted) {
      setStatus(edit->text() +":" + edit2->text());
  } else {
    setStatus("درسی ثبت نشد");
  }
}


void StudentInfo::addlesson(WString n, WString t){
  Dbo::Transaction t1(session);
  std::unique_ptr<lesson> les{new lesson};
  les->name = n.toUTF8();
  std::string sk= t.toUTF8();
  les->test = std::stol(sk);
  session.add(std::move(les));
  t1.commit();

}

// ثبت نام دانشجو
void StudentInfo::setStudent()
{
  WDialog dialog(" ثبت نام ");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);
  WLineEdit *edit = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit->setPlaceholderText("نام");
  WLineEdit *edit1 = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  edit1->setPlaceholderText("تاریخ تولد");
  WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("ثبت"));
  ok->setDefault(true);
  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("لغو"));
  ok1->setDefault(true);
  edit->setFocus();
  edit1->setFocus();
  ok->clicked().connect([&]{StudentInfo::addstudent(edit->text(),edit1->text());dialog.accept();});
  ok1->clicked().connect(&dialog, &WDialog::reject);
  WString s = edit->text();
  if (dialog.exec() == DialogCode::Accepted) {
        setStatus(edit->text() + "," + edit1->text());
  } else {
    setStatus("دانشجویی ثبت نشد");
  }
}

void StudentInfo::setStatus(const WString& result)
{
  status_->setText(result);
}

void StudentInfo::addstudent(WString n, WString b){
  Dbo::Transaction t(session);
  std::unique_ptr<Student> stu{new Student};
  std::string sk= b.toUTF8();
  stu->name = n.toUTF8();
  stu->born = std::stol(sk);
  session.add(std::move(stu));
  t.commit();
  count++;
}

void StudentInfo::print(void){
  WDialog dialog("لیست دانشجو");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);

  Dbo::Transaction t(session);
  typedef Dbo::collection< Dbo::ptr<Student> > Users;
  Users users = session.find<Student>();
  for (const dbo::ptr<Student> &user : users) {

    dialog.footer()->addWidget(Wt::cpp14::make_unique<Wt::WText>("<p> " + user->name + " " + std::to_string(user->born) + "</p>" ));
  }
  t.commit();

  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("خروج"));
  ok1->setDefault(true);
  ok1->clicked().connect(&dialog, &WDialog::reject);

  if (dialog.exec() == DialogCode::Accepted) {
        setStatus("همه دانشجو ها");
  } else {
    setStatus("خروج");
  }

}
void StudentInfo::printlesson(void){

  WDialog dialog("دروس");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);

  Dbo::Transaction t1(session);
  typedef Dbo::collection< Dbo::ptr<lesson> > Lessons;
  Lessons les = session.find<lesson>();
  if(les.size() > 0)
  for (const dbo::ptr<lesson> &less : les) {

    dialog.footer()->addWidget(Wt::cpp14::make_unique<Wt::WText>("<p> " + less->name + " " + std::to_string(less->test) + "</p>" ));
  }
  t1.commit();

  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("خروج"));
  ok1->setDefault(true);
  ok1->clicked().connect(&dialog, &WDialog::reject);

  if (dialog.exec() == DialogCode::Accepted) {
        setStatus("");
  } else {
    setStatus("خروج");
  }

}



void StudentInfo::printAllExam(void){

  WDialog dialog("امتحانات");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);
  Dbo::Transaction t1(session);
  typedef Dbo::collection< Dbo::ptr<exam> > Exams;
  Exams ex = session.find<exam>();
  if(ex.size() > 0)
  for (const dbo::ptr<exam> &less : ex) {

    dialog.footer()->addWidget(Wt::cpp14::make_unique<Wt::WText>("<p> " + less->name_s + "-"+ less->name_l+":"+ std::to_string(less->mark) + "</p>" ));
  }
  t1.commit();

  WPushButton *ok1 = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("خروج"));
  ok1->setDefault(true);
  ok1->clicked().connect(&dialog, &WDialog::reject);

  if (dialog.exec() == DialogCode::Accepted) {
        setStatus("");
  } else {
    setStatus("خروج");
  }

}


#endif //
