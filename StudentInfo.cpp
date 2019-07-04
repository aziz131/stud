
#include <iostream>
#include <memory>
#include <string>


#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include  <Wt/Dbo/Session.h>
#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>



#include "StudentInfo.hpp"

using namespace Wt;
std::unique_ptr<dbo::SqlConnectionPool> pool;

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return Wt::cpp14::make_unique<StudentInfo>(env, pool.get());
};

int main(int argc, char **argv)
{

  auto connection = Wt::cpp14::make_unique<dbo::backend::Sqlite3>("StudentDb");
  connection->setProperty("show-queries", "true");
   pool= Wt::cpp14::make_unique<dbo::FixedSqlConnectionPool>(std::move(connection), 10);
   return WRun(argc, argv, &createApplication);
}
