#include "CreateIndexDialog.h"
#include "ui_CreateIndexDialog.h"
#include "sqlitedb.h"

#include <QMessageBox>
#include <QPushButton>

CreateIndexDialog::CreateIndexDialog(DBBrowserDB* db, QWidget* parent)
    : QDialog(parent),
      pdb(db),
      ui(new Ui::CreateIndexDialog)
{
    // Create UI
    ui->setupUi(this);

    // Fill table combobox
    QList<DBBrowserObject> tables = pdb->objMap.values("table");
    for(int i=0; i < tables.count(); ++i)
        ui->comboTableName->addItem(tables.at(i).getname());
}

CreateIndexDialog::~CreateIndexDialog()
{
    delete ui;
}

void CreateIndexDialog::tableChanged(const QString& new_table)
{
    // And fill the table again
    QStringList fields = pdb->getTableFields(new_table);
    ui->tableIndexColumns->setRowCount(fields.size());
    for(int i=0; i < fields.size(); ++i)
    {
        // Put the name of the field in the first column
        QTableWidgetItem* name = new QTableWidgetItem(fields.at(i));
        name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->tableIndexColumns->setItem(i, 0, name);

        // Put a checkbox to enable usage in the index of this field in the second column
        QTableWidgetItem* enabled = new QTableWidgetItem("");
        enabled->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        enabled->setCheckState(Qt::Unchecked);
        ui->tableIndexColumns->setItem(i, 1, enabled);

        // And put a combobox to select the order in which to index the field in the last column
        QComboBox* order = new QComboBox(this);
        order->addItem("ASC");
        order->addItem("DESC");
        ui->tableIndexColumns->setCellWidget(i, 2, order);
    }
}

void CreateIndexDialog::checkInput()
{
    bool valid = true;
    if(ui->editIndexName->text().isEmpty() || ui->editIndexName->text().contains("`"))
        valid = false;

    int num_columns = 0;
    for(int i=0; i < ui->tableIndexColumns->rowCount(); ++i)
    {
        if(ui->tableIndexColumns->item(i, 1) && ui->tableIndexColumns->item(i, 1)->data(Qt::CheckStateRole) == Qt::Checked)
            num_columns++;
    }
    if(num_columns == 0)
        valid = false;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void CreateIndexDialog::accept()
{
    QString sql = QString("CREATE %1 INDEX `%2` ON `%3` (")
            .arg(ui->checkIndexUnique->isChecked() ? "UNIQUE" : "")
            .arg(ui->editIndexName->text())
            .arg(ui->comboTableName->currentText());

    for(int i=0; i < ui->tableIndexColumns->rowCount(); ++i)
    {
        if(ui->tableIndexColumns->item(i, 1)->data(Qt::CheckStateRole) == Qt::Checked)
        {
            sql.append(QString("`%1` %2,")
                       .arg(ui->tableIndexColumns->item(i, 0)->text())
                       .arg(qobject_cast<QComboBox*>(ui->tableIndexColumns->cellWidget(i, 2))->currentText()));
        }
    }
    sql.chop(1);    // Remove last comma
    sql.append(");");

    if(pdb->executeSQL(sql))
        QDialog::accept();
    else
        QMessageBox::warning(this, QApplication::applicationName(), tr("Creating the index failed:\n%1").arg(pdb->lastErrorMessage));
}
