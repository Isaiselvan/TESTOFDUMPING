#include "Interface.h" 


Interface::Interface()
{
 NodeShf = NULL;
 shfrql = NULL;
}

SharedHashFile * Interface::initialiseShf(std::string &nodepair)
{

    char  Shfnodelist[256];
    char  ShfFolder[] = SHF_DIR;
    char  Shfhopreqlst[256];
    int i = 1;
    sprintf(Shfnodelist, "NodeList"); 
    if(!NodeShf)
      NodeShf = new SharedHashFile;
//Creating Nodelist Table where <Key,Value> = <Nodepair, requestlisttablename>
    if(NodeShf && !NodeShf->IsAttached())
      {
        if(!NodeShf->AttachExisting(ShfFolder, Shfnodelist))
             NodeShf->Attach(ShfFolder, Shfnodelist, 1);

         NodeShf->SetIsLockable (1);
      }
        
//Retriving/creating the requestlisttablename
    NodeShf->MakeHash(nodepair.c_str(), nodepair.length()); 

    bzero(shf_val, sizeof(shf_val));
     if(NodeShf->GetKeyValCopy () )
      {
             sprintf(Shfhopreqlst,"%s", shf_val);
             std::cout << "Test " << Shfhopreqlst << std::endl;
      }else 
      {
        while (1)
        {
         bzero(Shfhopreqlst,sizeof(Shfhopreqlst));  
         sprintf(Shfhopreqlst,"node_%d",i++); 
            if(!NodeShf->AttachExisting(ShfFolder, Shfhopreqlst))
              {
                std::string keyV(Shfhopreqlst);
                NodeShf->AttachExisting(ShfFolder, Shfnodelist);
                NodeShf->MakeHash(nodepair.c_str(), nodepair.length()); 
                NodeShf->PutKeyVal(keyV.c_str(),keyV.length());
                break;
              }
        }
      }
   //NodeShf->Del();
     NodeShf->Detach();
//Creating/attaching the requestlisttable 
    if(!shfrql)
          shfrql =  new SharedHashFile;

    if(shfrql && !shfrql->IsAttached())
      {
         if(!shfrql->AttachExisting(ShfFolder, Shfhopreqlst))
             shfrql->Attach(ShfFolder, Shfhopreqlst, 0);    
          shfrql->SetIsLockable (1); 
          //shfrql->SetDataNeedFactor (250); 
      }

}
