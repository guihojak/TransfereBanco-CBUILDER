#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------

__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Botão para conectar ao banco
void __fastcall TForm1::btnConectarClick(TObject *Sender)
{
    if(OpenDialog1->Execute())
    {
        try
        {
            // Desconecta e reconecta para garantir
            IBDatabase1->Connected = false;
            IBDatabase1->DatabaseName = OpenDialog1->FileName;
            IBDatabase1->LoginPrompt = false;
            IBDatabase1->Params->Clear();
            IBDatabase1->Params->Add("user_name=SYSDBA");
            IBDatabase1->Params->Add("password=masterkey");

            IBDatabase1->Connected = true;

            Memo1->Lines->Add("Conectado ao banco: " + OpenDialog1->FileName);
        }
        catch(Exception &e)
        {
            Memo1->Lines->Add("Erro ao conectar: " + e.Message);
        }
    }
}

//---------------------------------------------------------------------------
// Função auxiliar para extrair o nome da tabela do comando SQL
AnsiString ExtractTableName(const AnsiString& SQLCommand) {
    int start = SQLCommand.Pos("TABLE") + 6; // Posição após "TABLE "
    int end = SQLCommand.Pos(" ADD");
    if (start > 0 && end > start) {
        return SQLCommand.SubString(start, end - start).Trim().UpperCase();
    }
    return "";
}

//---------------------------------------------------------------------------
// Função auxiliar para extrair o nome da coluna do comando SQL
AnsiString ExtractColumnName(const AnsiString& SQLCommand) {
    int start = SQLCommand.Pos("ADD") + 4; // Posição após "ADD "

    // Na sua versao, Pos() nao aceita segundo parametro
    AnsiString tempStr = SQLCommand.SubString(start, SQLCommand.Length() - start + 1);

    int end = tempStr.Pos(" ");
    if (end > 0) {
        return tempStr.SubString(1, end - 1).Trim().UpperCase();
    }

    return tempStr.Trim().UpperCase();
}

//---------------------------------------------------------------------------
// Botão para executar os scripts de atualização
void __fastcall TForm1::btnExecutarClick(TObject *Sender)
{
    if (!IBDatabase1->Connected)
    {
        ShowMessage("Conecte primeiro ao banco!");
        return;
    }

    TStringList *SQLCommands = new TStringList();
    
    // Adicione aqui todos os seus comandos ALTER TABLE
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD ALTERA_CURVAS_OD INTEGER;");
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD ALTERA_CURVAS_OE INTEGER;");
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD MOLDE_ESF_OD NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD MOLDE_CIL_OD NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD MOLDE_ESF_OE NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBPROCESSO ADD MOLDE_CIL_OE NUMERIC(5,2);");

    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_11 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_12 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_13 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_14 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_15 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_16 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_NEG_17 NUMERIC(5,2);");

    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_11 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_12 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_13 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_14 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_15 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_16 NUMERIC(5,2);");
    SQLCommands->Add("ALTER TABLE TBESPESSURAS ADD INT_POS_17 NUMERIC(5,2);");

    SQLCommands->Add("ALTER TABLE TBCOMPENSACAO ADD EIXO_ULTEX_ATIV INTEGER;");
    SQLCommands->Add("ALTER TABLE TBCOMPENSACAO ADD EIXO_ULTEX_DES INTEGER;");

    for (int i = 0; i < SQLCommands->Count; i++)
    {
        AnsiString sqlCommand = SQLCommands->Strings[i].Trim();
        AnsiString tableName = ExtractTableName(sqlCommand);
        AnsiString columnName = ExtractColumnName(sqlCommand);

        if (tableName.IsEmpty() || columnName.IsEmpty()) {
            Memo1->Lines->Add("Erro ao extrair nome da tabela/coluna do comando: " + sqlCommand);
            continue;
        }
        
        // Inicia uma nova transação para cada comando
        IBTransaction1->StartTransaction();
        IBSQL1->Transaction = IBTransaction1;
        IBQuery1->Transaction = IBTransaction1;

        try
        {
            // Verifica se a coluna já existe
            IBQuery1->Close();
            IBQuery1->SQL->Clear();
            IBQuery1->SQL->Add("SELECT RDB$FIELD_NAME FROM RDB$RELATION_FIELDS");
            IBQuery1->SQL->Add("WHERE RDB$RELATION_NAME = :TABLE_NAME AND RDB$FIELD_NAME = :COLUMN_NAME");
            IBQuery1->ParamByName("TABLE_NAME")->AsString = tableName;
            IBQuery1->ParamByName("COLUMN_NAME")->AsString = columnName;
            IBQuery1->Open();

            if (IBQuery1->IsEmpty())
            {
                // A coluna não existe, então execute o ALTER TABLE
                IBSQL1->Close();
                IBSQL1->SQL->Clear();
                IBSQL1->SQL->Add(sqlCommand);
                IBSQL1->ExecQuery();
                Memo1->Lines->Add("Executado: " + sqlCommand);
            }
            else
            {
                Memo1->Lines->Add("Ignorado (já existe): Coluna " + columnName + " na tabela " + tableName);
            }
            
            // Confirma a transação
            IBTransaction1->Commit();
        }
        catch(Exception &e)
        {
            // Em caso de erro, desfaz a transação e exibe a mensagem
            IBTransaction1->Rollback();
            Memo1->Lines->Add("Erro em: " + sqlCommand + " -> " + e.Message);
            // Parar a execução se um erro crítico ocorrer
            break;
        }
    }

    ShowMessage("Atualização do banco de dados concluída. Verifique o log para detalhes.");
    delete SQLCommands;
}



