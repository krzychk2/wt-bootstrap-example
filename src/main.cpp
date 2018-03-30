/*
 * main.cpp
 *
 *  Created on: 15.04.2017
 *      Author: krzychk2
 */


#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WLabel>
#include <Wt/WBootstrapTheme>
#include <Wt/WCssTheme>
#include <Wt/WEnvironment>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WMenu>
#include <Wt/WMenuItem>
#include <Wt/WPopupMenu>
#include <Wt/WNavigationBar>
#include <Wt/WStackedWidget>
#include <Wt/WImage>

//Charts
#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WPieChart>
#include <Wt/WStandardItemModel>
#include <Wt/WSvgImage>
#include <Wt/WPainter>
#include <Wt/WAbstractItemModel>
#include <Wt/WStandardItem>

// c++0x only, for std::bind
// #include <functional>
#include <memory>
#include <iostream>

using namespace Wt;
using namespace std;

void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
		 int numRows, bool firstLineIsHeaders=true)
{
  int csvRow = 0;

  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      typedef boost::tokenizer<boost::escaped_list_separator<char> >
	CsvTokenizer;
      CsvTokenizer tok(line);

      int col = 0;
      for (CsvTokenizer::iterator i = tok.begin();
	   i != tok.end(); ++i, ++col) {

	if (col >= model->columnCount())
	  model->insertColumns(model->columnCount(),
			       col + 1 - model->columnCount());

	if (firstLineIsHeaders && csvRow == 0)
	  model->setHeaderData(col, boost::any(Wt::WString::fromUTF8(*i)));
	else {
	  int dataRow = firstLineIsHeaders ? csvRow - 1 : csvRow;

	  if (numRows != -1 && dataRow >= numRows)
	    return;

	  if (dataRow >= model->rowCount())
	    model->insertRows(model->rowCount(),
			      dataRow + 1 - model->rowCount());

	  boost::any data;
	  std::string s = *i;

	  char *endptr;

	  if (s.empty())
	    data = boost::any();
	  else {
	    double d = strtod(s.c_str(), &endptr);
	    if (*endptr == 0)
	      data = boost::any(d);
	    else
	      data = boost::any(Wt::WString(s));
	  }

	  model->setData(dataRow, col, data);
	}
      }
    }

    ++csvRow;
  }
}


 /*
   * A standard item which converts text edits to numbers
   */
  class NumericItem : public WStandardItem {
  public:
    virtual NumericItem *clone() const {
      return new NumericItem();
    }

    virtual void setData(const boost::any &data, int role = UserRole) {
      boost::any dt;

      if (role == EditRole) {
	std::string s = Wt::asString(data).toUTF8();
	char *endptr;
	double d = strtod(s.c_str(), &endptr);
	if (*endptr == 0)
	  dt = boost::any(d);
	else
	  dt = data;
      }

      WStandardItem::setData(data, role);
    }
  };


/*
  * Reads a CSV file as an (editable) standard item model.
  */
WAbstractItemModel *readCsvFile(const std::string &fname,
				  WContainerWidget *parent)
{
   WStandardItemModel *model = new WStandardItemModel(0, 0, parent);
   model->setItemPrototype(new NumericItem());
   std::ifstream f(fname.c_str());

   if (f) {
     readFromCsv(f, model, true);

     for (int row = 0; row < model->rowCount(); ++row)
	for (int col = 0; col < model->columnCount(); ++col) {
	  model->item(row, col)->setFlags(ItemIsSelectable | ItemIsEditable);

	  /*
	    Example of tool tips (disabled here because they are not updated
	    when editing data)
	   */

	  /*
	  WString toolTip = asString(model->headerData(col)) + ": "
	    + asString(model->item(row, col)->data(DisplayRole), "%.f");
	  model->item(row, col)->setToolTip(toolTip);
	   */
	}

     return model;
   } else {
     WString error(WString::tr("error-missing-data"));
     //error.arg(fname, UTF8);
     new WText(error, parent);
     return 0;
   }
 }

WApplication *createApplication(const WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   *
   */
	//Wt::WApplication *app = new HelloApplication(env);

	Wt::WApplication* app = new Wt::WApplication(env);

    Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(app);
    bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);

    // load the sb-admin2 bootstrap3 (sub-)theme
    app->useStyleSheet("resources/themes/bootstrap/sb-admin2/dist/css/sb-admin-2.css");
    app->useStyleSheet("resources/themes/bootstrap/sb-admin2/vendor/metisMenu/metisMenu.min.css");
    app->useStyleSheet("resources/themes/bootstrap/sb-admin2/vendor/morrisjs/morris.css");
    app->useStyleSheet("resources/themes/bootstrap/sb-admin2/vendor/font-awesome/css/font-awesome.min.css");

    // Set title
    app->setTitle("Wt Bootstrap Test");

    // Create widgets

    /**
     * TOP Navigation Bar
     */
    Wt::WNavigationBar *topNavigationBar = new Wt::WNavigationBar(app->root());
    //topNavigationBar->setStyleClass("nav navbar-top-links navbar-right");
    topNavigationBar->setTitle("Sb Admin 2", "http://127.0.0.1/cos");
    topNavigationBar->setStyleClass("navbar navbar-default navbar-static-top");

    Wt::WContainerWidget *topLinksRight = new Wt::WContainerWidget();
    topLinksRight->setStyleClass("nav navbar-top-links navbar-right");
    Wt::WHBoxLayout *topLinksRightHBox = new Wt::WHBoxLayout();
    topLinksRight->setLayout(topLinksRightHBox);
    topLinksRightHBox->setSpacing(0);


    Wt::WPopupMenu *messagePopupMenu = new WPopupMenu();
    messagePopupMenu->setStyleClass("dropdown");

    messagePopupMenu->addItem("Message1");
    messagePopupMenu->addItem("Message2");
    messagePopupMenu->addItem("Message3");
    messagePopupMenu->addItem("Message4");

    Wt::WMenu *messageMenu = new Wt::WMenu();
    Wt::WMenuItem *messageItem = new Wt::WMenuItem("Message");
    messageItem->setMenu(messagePopupMenu);
    messageMenu->addItem(messageItem);
    messageMenu->setStyleClass("dropdown-toggle");


    Wt::WPopupMenu *taskPopupMenu = new WPopupMenu();

    taskPopupMenu->setStyleClass("dropdown");
    taskPopupMenu->addItem("Item1");
    taskPopupMenu->addItem("Item2");
    taskPopupMenu->addItem("Item3");
    taskPopupMenu->addItem("Item4");

    Wt::WMenu *taskMenu = new Wt::WMenu();
    Wt::WMenuItem *taskItem = new Wt::WMenuItem("Task");
    taskItem->setMenu(taskPopupMenu);
    taskMenu->addItem(taskItem);
    taskMenu->setStyleClass("dropdown-toggle");


    Wt::WPopupMenu *alertPopupMenu = new WPopupMenu();
    alertPopupMenu->setStyleClass("dropdown");

    alertPopupMenu->addItem("Alert1");
    alertPopupMenu->addItem("Alert2");
    alertPopupMenu->addItem("Alert3");
    alertPopupMenu->addItem("Alert4");


    Wt::WMenu *alertMenu = new Wt::WMenu();
    Wt::WMenuItem *alertItem = new Wt::WMenuItem("Alert");
    alertItem->setMenu(alertPopupMenu);
    alertMenu->addItem(alertItem);
    alertMenu->setStyleClass("dropdown-toggle");


    Wt::WPopupMenu *userPopupMenu = new WPopupMenu();
    userPopupMenu->setStyleClass("dropdown");

    userPopupMenu->addItem("User1");
    userPopupMenu->addItem("User2");
    userPopupMenu->addItem("User3");
    userPopupMenu->addItem("User4");

    Wt::WMenu *userMenu = new Wt::WMenu();
    Wt::WMenuItem *userItem = new Wt::WMenuItem("User");
    userItem->setMenu(userPopupMenu);
    userMenu->addItem(userItem);
    userMenu->setStyleClass("dropdown-toggle");


    //Dodajemy elementy
    topLinksRightHBox->addWidget(messageMenu);
    topLinksRightHBox->addWidget(taskMenu);
    topLinksRightHBox->addWidget(alertMenu);
    topLinksRightHBox->addWidget(userMenu);

    //topNavigationBar->addMenu(messageMenu, Wt::AlignRight);
    //topNavigationBar->addMenu(taskMenu, Wt::AlignRight);
    //topNavigationBar->addMenu(alertMenu, Wt::AlignRight);
    //topNavigationBar->addMenu(userMenu, Wt::AlignRight);
    topNavigationBar->addWidget(topLinksRight, Wt::AlignRight);


    //Left Navigation Bar
    Wt::WNavigationBar *leftNavigationBar = new Wt::WNavigationBar(app->root());
    leftNavigationBar->setStyleClass("navbar-default sidebar");

    Wt::WContainerWidget *sideMenu = new Wt::WContainerWidget();
    sideMenu->setStyleClass("nav in");
    sideMenu->setId("side-menu");


    Wt::WContainerWidget *customSearch = new Wt::WContainerWidget();
    customSearch->setStyleClass("sidebar-search");

    Wt::WLineEdit *searchLineEdit = new Wt::WLineEdit();
    Wt::WPushButton *searchButton = new Wt::WPushButton();
    searchButton->setText("<i class=\"fa fa-search\"></i>");
    searchButton->setTextFormat(Wt::XHTMLText);


    customSearch->addWidget(searchLineEdit);
    customSearch->addWidget(searchButton);


    Wt::WMenu *dashboardMenu = new Wt::WMenu();
    Wt::WMenuItem *dashboardItem = new Wt::WMenuItem("Dashboard");
    dashboardMenu->addItem(dashboardItem);
    //dashboardMenu->setStyleClass("nav in");

    Wt::WPopupMenu *chartsPopupMenu = new WPopupMenu();
    chartsPopupMenu->setStyleClass("dropdown");

    chartsPopupMenu->addItem("Chart1");
    chartsPopupMenu->addItem("Chart2");


    Wt::WMenu *chartsMenu = new Wt::WMenu();
    Wt::WMenuItem *chartsItem = new Wt::WMenuItem("Charts");
    chartsItem->setMenu(chartsPopupMenu);
    chartsMenu->addItem(chartsItem);
    //chartsMenu->setStyleClass("nav in");


    Wt::WMenu *tablesMenu = new Wt::WMenu();
    Wt::WMenuItem *tablesItem = new Wt::WMenuItem("Tables");
    tablesMenu->addItem(tablesItem);
    //tablesMenu->setStyleClass("nav in");

    Wt::WMenu *formsMenu = new Wt::WMenu();
    Wt::WMenuItem *formsItem = new Wt::WMenuItem("Forms");
    formsMenu->addItem(formsItem);
    //formsMenu->setStyleClass("nav in");

    Wt::WMenu *uiElementsMenu = new Wt::WMenu();
    Wt::WMenuItem *uiElementsItem = new Wt::WMenuItem("UI Elements");
    uiElementsMenu->addItem(uiElementsItem);
    //uiElementsMenu->setStyleClass("nav in");

    Wt::WMenu *multiLevelDropDownMenu = new Wt::WMenu();
    Wt::WMenuItem *multiLevelDropDownItem = new Wt::WMenuItem("Mulit-Level Dropdown");
    multiLevelDropDownMenu->addItem(multiLevelDropDownItem);
    //multiLevelDropDownMenu->setStyleClass("nav in");

    Wt::WMenu *samplePagesMenu = new Wt::WMenu();
    Wt::WMenuItem *samplePagesItem = new Wt::WMenuItem("Sample Pages");
    samplePagesMenu->addItem(samplePagesItem);
    //samplePagesMenu->setStyleClass("nav in");


    //Addign widgets
    sideMenu->addWidget(customSearch);
    sideMenu->addWidget(dashboardMenu);
    sideMenu->addWidget(chartsMenu);
    sideMenu->addWidget(tablesMenu);
    sideMenu->addWidget(formsMenu);
    sideMenu->addWidget(uiElementsMenu);
    sideMenu->addWidget(multiLevelDropDownMenu);
    sideMenu->addWidget(samplePagesMenu);

    leftNavigationBar->addWidget(sideMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(dashboardMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(chartsMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(tablesMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(formsMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(uiElementsMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(multiLevelDropDownMenu, Wt::AlignBottom);
    //leftNavigationBar->addMenu(samplePagesMenu, Wt::AlignBottom);

    //Page Wrapper (main page)
    Wt::WContainerWidget *mainPage = new Wt::WContainerWidget(app->root());
    mainPage->setId("page-wrapper");

    Wt::WContainerWidget *mainPageHeaderRow = new Wt::WContainerWidget(mainPage);
    mainPageHeaderRow->setStyleClass("row");

    //Page header
    Wt::WContainerWidget *mainPageHeader = new Wt::WContainerWidget(mainPageHeaderRow);
    mainPageHeader->setStyleClass("col-lg-12");

    Wt::WText *dashBoard = new Wt::WText(mainPageHeader);
    dashBoard->setText("<h1> Dashboard </h1>");
    dashBoard->setTextFormat(Wt::XHTMLText);
    dashBoard->setStyleClass("page-header");

    //Page header
    Wt::WContainerWidget *mainPagePanelsRow = new Wt::WContainerWidget(mainPage);
    mainPagePanelsRow->setStyleClass("row");

    //Main Page Comment Panel (make it as function??)
    {
    Wt::WContainerWidget *commentsPanelRow = new Wt::WContainerWidget(mainPagePanelsRow);
    commentsPanelRow->setStyleClass("col-lg-3 col-md-6");

    Wt::WContainerWidget *commentsPanel = new Wt::WContainerWidget(commentsPanelRow);
    commentsPanel->setStyleClass("panel panel-primary");

    Wt::WContainerWidget *commentsPanelHeading = new Wt::WContainerWidget(commentsPanel);
    commentsPanelHeading->setStyleClass("panel-heading");

    Wt::WContainerWidget *commentsPanelHeadingRow = new Wt::WContainerWidget(commentsPanelHeading);
    commentsPanelHeadingRow->setStyleClass("row");

    Wt::WContainerWidget *commentsPanelHeadingLeft = new Wt::WContainerWidget(commentsPanelHeadingRow);
    commentsPanelHeadingLeft->setStyleClass("col-xs-3");

    Wt::WText *commentsPanelIcon = new WText(commentsPanelHeadingLeft);
    commentsPanelIcon->setText("<i class=\"fa fa-comments fa-5x\"/>");
    commentsPanelIcon->setTextFormat(Wt::XHTMLText);

    Wt::WContainerWidget *commentsPanelHeadingRight = new Wt::WContainerWidget(commentsPanelHeadingRow);
    commentsPanelHeadingRight->setStyleClass("col-xs-9 text-right");

    Wt::WText *commentsPanelValue = new WText(commentsPanelHeadingRight);
    commentsPanelValue->setInline(false); //<div>
    commentsPanelValue->setText("26");
    commentsPanelValue->setStyleClass("huge");

    Wt::WText *commentsPanelInfo = new WText(commentsPanelHeadingRight);
    commentsPanelInfo->setInline(false); //<div>
    commentsPanelInfo->setText("New Comments!");

    Wt::WContainerWidget *commentsPanelFooter = new Wt::WContainerWidget(commentsPanel);
    commentsPanelFooter->setStyleClass("panel-footer");

    Wt::WText *commentsPanelViewDetails = new Wt::WText(commentsPanelFooter);
    commentsPanelViewDetails->setText("View Details");
    commentsPanelViewDetails->setStyleClass("pull-left");

    Wt::WText *commentsPanelViewDetailsArrow = new Wt::WText(commentsPanelFooter);
    commentsPanelViewDetailsArrow->setText("<i class=\"fa fa-arrow-circle-right\"/>");
    commentsPanelViewDetailsArrow->setStyleClass("pull-right");

    Wt::WContainerWidget *commentsPanelFooterClearFix = new Wt::WContainerWidget(commentsPanelFooter);
    commentsPanelFooterClearFix->setStyleClass("clearfix");
    }
    // End Main Page Comment Panel (make it as function ??)

    //Main Page Tasks Panel (make it as function??)
    {
    Wt::WContainerWidget *tasksPanelRow = new Wt::WContainerWidget(mainPagePanelsRow);
    tasksPanelRow->setStyleClass("col-lg-3 col-md-6");

    Wt::WContainerWidget *tasksPanel = new Wt::WContainerWidget(tasksPanelRow);
    tasksPanel->setStyleClass("panel panel-green");

    Wt::WContainerWidget *tasksPanelHeading = new Wt::WContainerWidget(tasksPanel);
    tasksPanelHeading->setStyleClass("panel-heading");

    Wt::WContainerWidget *tasksPanelHeadingRow = new Wt::WContainerWidget(tasksPanelHeading);
    tasksPanelHeadingRow->setStyleClass("row");

    Wt::WContainerWidget *tasksPanelHeadingLeft = new Wt::WContainerWidget(tasksPanelHeadingRow);
    tasksPanelHeadingLeft->setStyleClass("col-xs-3");

    Wt::WText *tasksPanelIcon = new WText(tasksPanelHeadingLeft);
    tasksPanelIcon->setText("<i class=\"fa fa-tasks fa-5x\"/>");
    tasksPanelIcon->setTextFormat(Wt::XHTMLText);

    Wt::WContainerWidget *tasksPanelHeadingRight = new Wt::WContainerWidget(tasksPanelHeadingRow);
    tasksPanelHeadingRight->setStyleClass("col-xs-9 text-right");

    Wt::WText *tasksPanelValue = new WText(tasksPanelHeadingRight);
    tasksPanelValue->setInline(false); //<div>
    tasksPanelValue->setText("26");
    tasksPanelValue->setStyleClass("huge");

    Wt::WText *tasksPanelInfo = new WText(tasksPanelHeadingRight);
    tasksPanelInfo->setInline(false); //<div>
    tasksPanelInfo->setText("New Tasks!");

    Wt::WContainerWidget *tasksPanelFooter = new Wt::WContainerWidget(tasksPanel);
    tasksPanelFooter->setStyleClass("panel-footer");

    Wt::WText *tasksPanelViewDetails = new Wt::WText(tasksPanelFooter);
    tasksPanelViewDetails->setText("View Details");
    tasksPanelViewDetails->setStyleClass("pull-left");

    Wt::WText *tasksPanelViewDetailsArrow = new Wt::WText(tasksPanelFooter);
    tasksPanelViewDetailsArrow->setText("<i class=\"fa fa-arrow-circle-right\"/>");
    tasksPanelViewDetailsArrow->setStyleClass("pull-right");

    Wt::WContainerWidget *tasksPanelFooterClearFix = new Wt::WContainerWidget(tasksPanelFooter);
    tasksPanelFooterClearFix->setStyleClass("clearfix");
    }

    //End Main Page Tasks Panel (make it as function??)

    //Main Page Orders Panel (make it as function??)
    {
    Wt::WContainerWidget *ordersPanelRow = new Wt::WContainerWidget(mainPagePanelsRow);
    ordersPanelRow->setStyleClass("col-lg-3 col-md-6");

    Wt::WContainerWidget *ordersPanel = new Wt::WContainerWidget(ordersPanelRow);
    ordersPanel->setStyleClass("panel panel-yellow");

    Wt::WContainerWidget *ordersPanelHeading = new Wt::WContainerWidget(ordersPanel);
    ordersPanelHeading->setStyleClass("panel-heading");

    Wt::WContainerWidget *ordersPanelHeadingRow = new Wt::WContainerWidget(ordersPanelHeading);
    ordersPanelHeadingRow->setStyleClass("row");

    Wt::WContainerWidget *ordersPanelHeadingLeft = new Wt::WContainerWidget(ordersPanelHeadingRow);
    ordersPanelHeadingLeft->setStyleClass("col-xs-3");

    Wt::WText *ordersPanelIcon = new WText(ordersPanelHeadingLeft);
    ordersPanelIcon->setText("<i class=\"fa fa-shopping-cart fa-5x\"/>");
    ordersPanelIcon->setTextFormat(Wt::XHTMLText);

    Wt::WContainerWidget *ordersPanelHeadingRight = new Wt::WContainerWidget(ordersPanelHeadingRow);
    ordersPanelHeadingRight->setStyleClass("col-xs-9 text-right");

    Wt::WText *ordersPanelValue = new WText(ordersPanelHeadingRight);
    ordersPanelValue->setInline(false); //<div>
    ordersPanelValue->setText("26");
    ordersPanelValue->setStyleClass("huge");

    Wt::WText *ordersPanelInfo = new WText(ordersPanelHeadingRight);
    ordersPanelInfo->setInline(false); //<div>
    ordersPanelInfo->setText("New Tasks!");

    Wt::WContainerWidget *ordersPanelFooter = new Wt::WContainerWidget(ordersPanel);
    ordersPanelFooter->setStyleClass("panel-footer");

    Wt::WText *ordersPanelViewDetails = new Wt::WText(ordersPanelFooter);
    ordersPanelViewDetails->setText("View Details");
    ordersPanelViewDetails->setStyleClass("pull-left");

    Wt::WText *ordersPanelViewDetailsArrow = new Wt::WText(ordersPanelFooter);
    ordersPanelViewDetailsArrow->setText("<i class=\"fa fa-arrow-circle-right\"/>");
    ordersPanelViewDetailsArrow->setStyleClass("pull-right");

    Wt::WContainerWidget *ordersPanelFooterClearFix = new Wt::WContainerWidget(ordersPanelFooter);
    ordersPanelFooterClearFix->setStyleClass("clearfix");
    }

    //End Main Page Orders Panel (make it as function??)

    //Main Page Support Panel (make it as function??)

    {
    Wt::WContainerWidget *supportPanelRow = new Wt::WContainerWidget(mainPagePanelsRow);
    supportPanelRow->setStyleClass("col-lg-3 col-md-6");

    Wt::WContainerWidget *supportPanel = new Wt::WContainerWidget(supportPanelRow);
    supportPanel->setStyleClass("panel panel-red");

    Wt::WContainerWidget *supportPanelHeading = new Wt::WContainerWidget(supportPanel);
    supportPanelHeading->setStyleClass("panel-heading");

    Wt::WContainerWidget *supportPanelHeadingRow = new Wt::WContainerWidget(supportPanelHeading);
    supportPanelHeadingRow->setStyleClass("row");

    Wt::WContainerWidget *supportPanelHeadingLeft = new Wt::WContainerWidget(supportPanelHeadingRow);
    supportPanelHeadingLeft->setStyleClass("col-xs-3");

    Wt::WText *supportPanelIcon = new WText(supportPanelHeadingLeft);
    supportPanelIcon->setText("<i class=\"fa fa-support fa-5x\"/>");
    supportPanelIcon->setTextFormat(Wt::XHTMLText);

    Wt::WContainerWidget *supportPanelHeadingRight = new Wt::WContainerWidget(supportPanelHeadingRow);
    supportPanelHeadingRight->setStyleClass("col-xs-9 text-right");

    Wt::WText *supportPanelValue = new WText(supportPanelHeadingRight);
    supportPanelValue->setInline(false); //<div>
    supportPanelValue->setText("26");
    supportPanelValue->setStyleClass("huge");

    Wt::WText *supportPanelInfo = new WText(supportPanelHeadingRight);
    supportPanelInfo->setInline(false); //<div>
    supportPanelInfo->setText("New Tickets!");

    Wt::WContainerWidget *supportPanelFooter = new Wt::WContainerWidget(supportPanel);
    supportPanelFooter->setStyleClass("panel-footer");

    Wt::WText *supportPanelViewDetails = new Wt::WText(supportPanelFooter);
    supportPanelViewDetails->setText("View Details");
    supportPanelViewDetails->setStyleClass("pull-left");

    Wt::WText *supportPanelViewDetailsArrow = new Wt::WText(supportPanelFooter);
    supportPanelViewDetailsArrow->setText("<i class=\"fa fa-arrow-circle-right\"/>");
    supportPanelViewDetailsArrow->setStyleClass("pull-right");

    Wt::WContainerWidget *supportPanelFooterClearFix = new Wt::WContainerWidget(supportPanelFooter);
    supportPanelFooterClearFix->setStyleClass("clearfix");
    }
    //End Main Page Support Panel (make it as function??)

    // Main Page Content Row
    Wt::WContainerWidget *mainPageContentRow = new Wt::WContainerWidget(mainPage);
    mainPageContentRow->setStyleClass("row");

    // Main Page 8Lg
    Wt::WContainerWidget *mainPageCol8 = new Wt::WContainerWidget(mainPageContentRow);
    mainPageCol8->setStyleClass("col-lg-8");

    // Main Page 4LG
    Wt::WContainerWidget *mainPageCol4 = new Wt::WContainerWidget(mainPageContentRow);
    mainPageCol4->setStyleClass("col-lg-4");


    //Area chart example
    {
        Wt::WContainerWidget *areaChartPanel = new Wt::WContainerWidget(mainPageCol8);
        areaChartPanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(areaChartPanel);
        panelHeader->setStyleClass("panel-heading");

        Wt::WText *chatPanelHeaderIcon = new WText(panelHeader);
        chatPanelHeaderIcon->setText("<i class=\"fa fa-bar-chart-o fa-fw\"/>");
        chatPanelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *chatPanelHeaderText = new WText(panelHeader);
        chatPanelHeaderText->setText("Area Chart Example");

        Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(areaChartPanel);
        panelBody->setStyleClass("panel-body");

        WStandardItemModel *model = new WStandardItemModel(40, 2, panelBody);
        model->setItemPrototype(new NumericItem());
        model->setHeaderData(0, WString("X"));
        model->setHeaderData(1, WString("Y = sin(X)"));

        for (unsigned i = 0; i < 40; ++i) {
          double x = (static_cast<double>(i) - 20) / 4;

          model->setData(i, 0, x);
          model->setData(i, 1, sin(x));
        }

        /*
         * Create the scatter plot.
         */
        Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(panelBody);
        chart->setModel(model);        // set the model
        chart->setXSeriesColumn(0);    // set the column that holds the X data
        chart->setLegendEnabled(true); // enable the legend
        //chart->setZoomEnabled(true);
        //chart->setPanEnabled(true);
        //chart->setCrosshairEnabled(true);

        //chart->setBackground(WColor(200,200,200));

        chart->setType(Wt::Chart::ScatterPlot);   // set type to ScatterPlot

        // Typically, for mathematical functions, you want the axes to cross
        // at the 0 mark:
        chart->axis(Wt::Chart::XAxis).setLocation(Wt::Chart::ZeroValue);
        chart->axis(Wt::Chart::YAxis).setLocation(Wt::Chart::ZeroValue);

        chart->resize(1000, 310);

        // Add the curves
        Wt::Chart::WDataSeries s(1, Wt::Chart::CurveSeries);
        s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        chart->addSeries(s);


        chart->setMargin(10, Top | Bottom);            // add margin vertically
        chart->setMargin(WLength::Auto, Left | Right); // center horizontally

        //Wt::WContainerWidget *panelFooter = new Wt::WContainerWidget(areaChartPanel);
        //panelFooter->setStyleClass("panel-footer");

    }
    //End area chart example

    //Bar chart example
    {
        Wt::WContainerWidget *barChartPanel = new Wt::WContainerWidget(mainPageCol8);
        barChartPanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(barChartPanel);
        panelHeader->setStyleClass("panel-heading");

        Wt::WText *chatPanelHeaderIcon = new WText(panelHeader);
        chatPanelHeaderIcon->setText("<i class=\"fa fa-bar-chart-o fa-fw\"/>");
        chatPanelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *chatPanelHeaderText = new WText(panelHeader);
        chatPanelHeaderText->setText("Bar Chart Example");

        Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(barChartPanel);
        panelBody->setStyleClass("panel-body");

        /*
         *
         */
        WAbstractItemModel *model = readCsvFile("/home/krzychk2/tmp/category.csv", panelBody);

 	 	Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(panelBody);
  	  	chart->setModel(model);        // set the model
  	  	chart->setXSeriesColumn(0);    // set the column that holds the categories
		chart->setLegendEnabled(true); // enable the legend

        chart->resize(1000, 310);

        /*
         * Add all (but first) column as bar series
         */
        for (int i = 1; i < model->columnCount(); ++i) {
          Wt::Chart::WDataSeries s(i, Wt::Chart::BarSeries);
          s.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
          chart->addSeries(s);
        }


        //Wt::WContainerWidget *panelFooter = new Wt::WContainerWidget(barChartPanel);
        //panelFooter->setStyleClass("panel-footer");

    }
    //End Bar chart example

    //Donut chart example
    {
        Wt::WContainerWidget *donutChartPanel = new Wt::WContainerWidget(mainPageCol4);
        donutChartPanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(donutChartPanel);
        panelHeader->setStyleClass("panel-heading");

        Wt::WText *panelHeaderIcon = new WText(panelHeader);
        panelHeaderIcon->setText("<i class=\"fa fa-bar-chart-o fa-fw\"/>");
        panelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *panelHeaderText = new WText(panelHeader);
        panelHeaderText->setText("Donut Chart Example");

        Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(donutChartPanel);
        panelBody->setStyleClass("panel-body");

        WStandardItemModel *model = new WStandardItemModel(panelBody);
        model->setItemPrototype(new NumericItem());

        //headers
        model->insertColumns(model->columnCount(), 2);
        model->setHeaderData(0, WString("Item"));
        model->setHeaderData(1, WString("Sales"));

        //data
        model->insertRows(model->rowCount(), 6);
        int row = 0;
        model->setData(row, 0, WString("Blueberry"));
        model->setData(row, 1, 120);
        // model->setData(row, 1, WString("Blueberry"), ToolTipRole);
        row++;
        model->setData(row, 0, WString("Cherry"));
        model->setData(row, 1, 30);
        row++;
        model->setData(row, 0, WString("Apple"));
        model->setData(row, 1, 260);
        row++;
        model->setData(row, 0, WString("Boston Cream"));
        model->setData(row, 1, 160);
        row++;
        model->setData(row, 0, WString("Other"));
        model->setData(row, 1, 40);
        row++;
        model->setData(row, 0, WString("Vanilla Cream"));
        model->setData(row, 1, 120);
        row++;

        //set all items to be editable and selectable
        for (int row = 0; row < model->rowCount(); ++row)
          for (int col = 0; col < model->columnCount(); ++col)
            model->item(row, col)->setFlags(ItemIsSelectable | ItemIsEditable);

        /*
         * Create the pie chart.
         */
        Wt::Chart::WPieChart *chart = new Wt::Chart::WPieChart(panelBody);
        chart->setModel(model);       // set the model
        chart->setLabelsColumn(0);    // set the column that holds the labels
        chart->setDataColumn(1);      // set the column that holds the data

        // configure location and type of labels
        chart->setDisplayLabels(Wt::Chart::Outside | Wt::Chart::TextLabel | Wt::Chart::TextPercentage);

        // enable a 3D and shadow effect
        chart->setPerspectiveEnabled(true, 0.2);
        chart->setShadowEnabled(true);

        // explode the first item
        chart->setExplode(0, 0.3);

        chart->setMargin(10, Top | Bottom);            // add margin vertically
        chart->setMargin(WLength::Auto, Left | Right); // center horizontally

        chart->resize(470, 300);
        //Wt::WContainerWidget *donutChartPanelFooter = new Wt::WContainerWidget(donutChartPanel);
        //donutChartPanelFooter->setStyleClass("panel-footer");

    }
    //End donut chart example

    //Responsive timeline
    {
        Wt::WContainerWidget *responsiveTimelinePanel = new Wt::WContainerWidget(mainPageCol8);
        responsiveTimelinePanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *responsivePanelHeader = new Wt::WContainerWidget(responsiveTimelinePanel);
        responsivePanelHeader->setStyleClass("panel-heading");

        Wt::WText *responsivePanelHeaderIcon = new WText(responsivePanelHeader);
        responsivePanelHeaderIcon->setText("<i class=\"fa fa-clock-o fa-fw\"/>");
        responsivePanelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *responsivePanelHeaderText = new WText(responsivePanelHeader);
        responsivePanelHeaderText->setText("Responsive Timeline");

        Wt::WContainerWidget *responsivePanelBody = new Wt::WContainerWidget(responsiveTimelinePanel);
        responsivePanelBody->setStyleClass("panel-body");

        Wt::WContainerWidget *responsiveMessages = new Wt::WContainerWidget(responsivePanelBody);
        responsiveMessages->setStyleClass("timeline");
        responsiveMessages->setList(true);

        Wt::WContainerWidget *leftMSG = new Wt::WContainerWidget(responsiveMessages);
        //leftMSG->setStyleClass("left clearfix");
        {
            Wt::WContainerWidget *badge = new Wt::WContainerWidget(leftMSG);
            badge->setStyleClass("timeline-badge");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-check\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(leftMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");

            Wt::WText *panelHeaderTitle = new Wt::WText(panelHeader);
            panelHeaderTitle->setText("<h4 class=\"timeline-title\">Lorem ipsum donor</h4>");
            panelHeaderTitle->setTextFormat(Wt::XHTMLText);

            Wt::WText *panelTextMuted = new Wt::WText(panelHeader);
            panelTextMuted->setText("<small class=\"timeline-title\"><i class=\"fa fa-clock-o\"/>12 mins ago on Twitter</small>");
            panelTextMuted->setTextFormat(Wt::XHTMLText);


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

        Wt::WContainerWidget *rightMSG = new Wt::WContainerWidget(responsiveMessages);
        rightMSG->setStyleClass("timeline-inverted");
        {
            Wt::WContainerWidget *badge = new Wt::WContainerWidget(rightMSG);
            badge->setStyleClass("timeline-badge warning");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-credit-card\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(rightMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");

            Wt::WText *panelHeaderTitle =  new Wt::WText(panelHeader);
            panelHeaderTitle->setStyleClass("timeline-title");
            panelHeaderTitle->setText("Lorem ipsum dolor");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

        //Wt::WContainerWidget
		leftMSG = new Wt::WContainerWidget(responsiveMessages);
        //leftMSG->setStyleClass("left clearfix");
        {
            Wt::WContainerWidget *badge = new Wt::WContainerWidget(leftMSG);
            badge->setStyleClass("timeline-badge danger");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-bomb\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(leftMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

        //Wt::WContainerWidget
		rightMSG = new Wt::WContainerWidget(responsiveMessages);
        rightMSG->setStyleClass("timeline-inverted");
        {
            /*Wt::WContainerWidget *badge = new Wt::WContainerWidget(rightMSG);
            badge->setStyleClass("timeline-badge warning");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-bomb\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);*/

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(rightMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");

            Wt::WText *panelHeaderTitle =  new Wt::WText(panelHeader);
            panelHeaderTitle->setStyleClass("timeline-title");
            panelHeaderTitle->setText("Lorem ipsum dolor");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

        //Wt::WContainerWidget
		leftMSG = new Wt::WContainerWidget(responsiveMessages);
        //leftMSG->setStyleClass("left clearfix");
        {
            Wt::WContainerWidget *badge = new Wt::WContainerWidget(leftMSG);
            badge->setStyleClass("timeline-badge info");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-save\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(leftMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

        //Wt::WContainerWidget
		leftMSG = new Wt::WContainerWidget(responsiveMessages);
        //leftMSG->setStyleClass("left clearfix");
        {
            /*Wt::WContainerWidget *badge = new Wt::WContainerWidget(leftMSG);
            badge->setStyleClass("timeline-badge info");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-save\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);*/

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(leftMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }

		rightMSG = new Wt::WContainerWidget(responsiveMessages);
        rightMSG->setStyleClass("timeline-inverted");
        {
            Wt::WContainerWidget *badge = new Wt::WContainerWidget(rightMSG);
            badge->setStyleClass("timeline-badge success");

            Wt::WText *badgeIcon = new WText(badge);
            badgeIcon->setText("<i class=\"fa fa-graduation-cap\"/>");
            badgeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *panel = new Wt::WContainerWidget(rightMSG);
            panel->setStyleClass("timeline-panel");

            Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(panel);
            panelHeader->setStyleClass("timeline-heading");

            Wt::WText *panelHeaderTitle =  new Wt::WText(panelHeader);
            panelHeaderTitle->setStyleClass("timeline-title");
            panelHeaderTitle->setText("Lorem ipsum dolor");


            Wt::WContainerWidget *panelBody = new Wt::WContainerWidget(panel);
            panelBody->setStyleClass("timeline-body");

            Wt::WText *bodyText = new Wt::WText(panelBody);
            bodyText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit. Libero laboriosam dolor perspiciatis omnis exercitationem. Beatae, officia pariatur? Est cum veniam excepturi. Maiores praesentium, porro voluptas suscipit facere rem dicta, debitis.");

        }


        //Wt::WContainerWidget *responsivePanelFooter = new Wt::WContainerWidget(responsiveTimelinePanel);
        //responsivePanelFooter->setStyleClass("panel-footer");

    }

    //Notification Panel
    {
        Wt::WContainerWidget *notificationPanel = new Wt::WContainerWidget(mainPageCol4);
        notificationPanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *panelHeader = new Wt::WContainerWidget(notificationPanel);
        panelHeader->setStyleClass("panel-heading");

        Wt::WText *panelHeaderIcon = new WText(panelHeader);
        panelHeaderIcon->setText("<i class=\"fa fa-bell fa-fw\"/>");
        panelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *panelHeaderText = new WText(panelHeader);
        panelHeaderText->setText("Notification Panel");

        Wt::WContainerWidget *notificationPanelBody = new Wt::WContainerWidget(notificationPanel);
        notificationPanelBody->setStyleClass("panel-body");

        Wt::WContainerWidget *listGroup = new Wt::WContainerWidget(notificationPanelBody);
        listGroup->setStyleClass("list-group");

        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

        	Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-comment fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("New Comment");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");

            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }

        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-twitter fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("3 New Followers");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }

        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-envelope fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("Message Sent");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");

            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }
        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-tasks fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("New Task");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }
        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-upload fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("New Upload");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }
        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-bolt fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("Server Crashed!");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }
        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

        	Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-warning fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("Server not responding");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }
        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-shopping-cart fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("New Order Placed");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }

        {
            Wt::WContainerWidget *listGroupItem = new Wt::WContainerWidget(listGroup);
            listGroupItem->setStyleClass("list-group-item");

            Wt::WText *icon = new Wt::WText(listGroupItem);
        	icon->setText("<i class=\"fa fa-money fa-fw\"/>");
            icon->setTextFormat(Wt::XHTMLText);

            Wt::WText *text = new Wt::WText(listGroupItem);
        	text->setText("Payment Received");
            text->setTextFormat(Wt::XHTMLText);

            Wt::WContainerWidget *time = new Wt::WContainerWidget(listGroupItem);
            time->setInline(true);
            time->setStyleClass("pull-right text-muted small");
            Wt::WText *timeText = new Wt::WText(time);
        	timeText->setText("<em>4 minutes ago</em>");
            timeText->setTextFormat(Wt::XHTMLText);

        }

        Wt::WText *text = new Wt::WText(notificationPanelBody);
    	text->setText("<a href=\"#\" class=\"btn btn-default btn-block\">View All Alerts</a>");
        text->setTextFormat(Wt::XHTMLText);

        //Wt::WContainerWidget *notificationPanelFooter = new Wt::WContainerWidget(notificationPanel);
        //notificationPanelFooter->setStyleClass("panel-footer");

    }
    //End notification Panel

    //Chat
    {
        Wt::WContainerWidget *chatPanel = new Wt::WContainerWidget(mainPageCol4);
        chatPanel->setStyleClass("chat-panel panel panel-default");

        Wt::WContainerWidget *chatPanelHeader = new Wt::WContainerWidget(chatPanel);
        chatPanelHeader->setStyleClass("panel-heading");

        Wt::WText *chatPanelHeaderIcon = new WText(chatPanelHeader);
        chatPanelHeaderIcon->setText("<i class=\"fa fa-comments fa-fw\"/>");
        chatPanelHeaderIcon->setTextFormat(Wt::XHTMLText);

        Wt::WText *chatPanelHeaderText = new WText(chatPanelHeader);
        chatPanelHeaderText->setText("Chat");

        Wt::WContainerWidget *chatPanelButtonGroup = new Wt::WContainerWidget(chatPanelHeader);
        chatPanelButtonGroup->setStyleClass("btn-group pull-right");

        Wt::WPushButton *chatPanelHeaderButton = new WPushButton(chatPanelButtonGroup);
        chatPanelHeaderButton->setStyleClass("btn btn-default btn-xs dropdown-toggle");
        //chatPanelHeaderButton->setText("<i class=\"fa fa-chevron-down\"/>");
        //chatPanelHeaderButton->setTextFormat(Wt::XHTMLText);

        Wt::WPopupMenu *chatPopupMenu = new WPopupMenu();
        chatPopupMenu->setStyleClass("dropdown");

        chatPopupMenu->addItem("<i class=\"fa fa-refresh fa-fw\"></i> Refresh");
        chatPopupMenu->addItem("<i class=\"fa fa-check-circle fa-fw\"></i> Avaliable");
        chatPopupMenu->addItem("<i class=\"fa fa-times fa-fw\"></i> Busy");
        chatPopupMenu->addItem("<i class=\"fa fa-clock-o fa-fw\"></i> Away");
        chatPopupMenu->addSeparator();
        chatPopupMenu->addItem("<i class=\"fa fa-sign-out fa-fw\"></i> Sign Out");

        chatPanelHeaderButton->setMenu(chatPopupMenu);


        Wt::WContainerWidget *chatPanelBody = new Wt::WContainerWidget(chatPanel);
        chatPanelBody->setStyleClass("panel-body");

        Wt::WContainerWidget *chatPanelFooter = new Wt::WContainerWidget(chatPanel);
        chatPanelFooter->setStyleClass("panel-footer");

        Wt::WContainerWidget *chatPanelFooterInputGroup = new Wt::WContainerWidget(chatPanelFooter);
        chatPanelFooterInputGroup->setStyleClass("input-group");

        Wt::WText *chatInput = new Wt::WText(chatPanelFooterInputGroup);
        chatInput->setStyleClass("form-control input-sm");
        chatInput->setText("Type your message here...");

        Wt::WContainerWidget *chatPanelFooterInputGroupBtn = new Wt::WContainerWidget(chatPanelFooterInputGroup);
        chatPanelFooterInputGroupBtn->setInline(true);
        chatPanelFooterInputGroupBtn->setStyleClass("input-group-btn");

        Wt::WPushButton *chatButton = new Wt::WPushButton("Send");
        chatButton->setStyleClass("btn btn-warning btn-sm");
        chatPanelFooterInputGroupBtn->addWidget(chatButton);

        Wt::WContainerWidget *chatMessages = new Wt::WContainerWidget(chatPanelBody);
        chatMessages->setStyleClass("chat");
        chatMessages->setList(true);

        Wt::WContainerWidget *leftMSG = new Wt::WContainerWidget(chatMessages);
        leftMSG->setStyleClass("left clearfix");
        {
        	Wt::WContainerWidget *avatar = new Wt::WContainerWidget(leftMSG);
        	avatar->setStyleClass("chat-img pull-left");
        	Wt::WImage *avatarIMG = new Wt::WImage("images/johnny_cash.png");
        	avatarIMG->setStyleClass("img-circle");
        	leftMSG->addWidget(avatarIMG);

        	Wt::WContainerWidget *msgBody = new Wt::WContainerWidget(leftMSG);
        	msgBody->setStyleClass("chat-body clearfix");

        	Wt::WContainerWidget *msgHeader = new Wt::WContainerWidget(msgBody);
        	msgHeader->setStyleClass("header");

        	//Strong
        	Wt::WText *login = new Wt::WText(msgHeader);
        	login->setStyleClass("primary-font");
        	login->setText("<strong class=\"primary-font\">Jack Sparrow</strong>");
            login->setTextFormat(Wt::XHTMLText);

        	Wt::WContainerWidget *timeTextMuted = new Wt::WContainerWidget(msgHeader);
        	timeTextMuted->setStyleClass("text-muted");

            Wt::WText *timeIcon = new WText(timeTextMuted);
            timeIcon->setText("<i class=\"fa fa-clock-o fa-fw\"/>");
            timeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WText *timeText = new WText(timeTextMuted);
            timeText->setText("12 mins");
            timeText->setStyleClass("pull-left text-muted");

            Wt::WText *msgText = new WText(msgBody);
            msgText->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur bibendum ornare dolor, quis ullamcorper ligula sodales.");
        }

        Wt::WContainerWidget *rightMSG = new Wt::WContainerWidget(chatMessages);
        rightMSG->setStyleClass("right clearfix");
        {
        	Wt::WContainerWidget *avatar = new Wt::WContainerWidget(rightMSG);
        	avatar->setStyleClass("chat-img pull-left");

        	Wt::WImage *avatarIMG = new Wt::WImage("images/johnny_cash.png");
        	avatarIMG->setStyleClass("img-circle");
        	rightMSG->addWidget(avatarIMG);

        	Wt::WContainerWidget *msgBody = new Wt::WContainerWidget(rightMSG);
        	msgBody->setStyleClass("chat-body clearfix");

        	Wt::WContainerWidget *msgHeader = new Wt::WContainerWidget(msgBody);
        	msgHeader->setStyleClass("header");

        	//Strong
        	Wt::WText *login = new Wt::WText(msgHeader);
        	login->setStyleClass("pull-right primary-font");
        	login->setText("Jack Sparrow");

        	Wt::WContainerWidget *timeTextMuted = new Wt::WContainerWidget(msgHeader);
        	timeTextMuted->setStyleClass("pull-left text-muted");

            Wt::WText *timeIcon = new WText(timeTextMuted);
            timeIcon->setText("<i class=\"fa fa-clock-o fa-fw\"/>");
            timeIcon->setTextFormat(Wt::XHTMLText);

            Wt::WText *timeText = new WText(timeTextMuted);
            timeText->setText("12 mins");
            timeText->setStyleClass("pull-left text-muted");

            Wt::WText *msgText = new WText(msgBody);
            msgText->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur bibendum ornare dolor, quis ullamcorper ligula sodales.");
        }


    }

    return app;
}

int main(int argc, char **argv)
{
  /*
   * Your main method may set up some shared resources, but should then
   * start the server application (FastCGI or httpd) that starts listening
   * for requests, and handles all of the application life cycles.
   *
   * The last argument to WRun specifies the function that will instantiate
   * new application objects. That function is executed when a new user surfs
   * to the Wt application, and after the library has negotiated browser
   * support. The function should return a newly instantiated application
   * object.
   */
  return WRun(argc, argv, &createApplication);
}



