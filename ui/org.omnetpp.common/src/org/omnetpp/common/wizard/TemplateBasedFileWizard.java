package org.omnetpp.common.wizard;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IPath;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.dialogs.WizardNewFileCreationPage;
import org.eclipse.ui.ide.IDE;
import org.omnetpp.common.CommonPlugin;

/**
 * Abstract base class for template-based "New xxx File..." wizards. 
 * 
 * Extends TemplateBasedWizard with a file selection first page, adds the 
 * "wizardType" and "newFileName" variables into the context, and opens 
 * the created file after completion. 
 * 
 * Subclasses are expected to override addPage() to configure the 
 * title, description etc, and provide a getTemplates() method. 
 * 
 * This wizard does not prevent the content template from creating other
 * files than "newFileName", so it can be used for e.g. simple modules
 * (ned + cc + h) as well. 
 * 
 * @author Andras
 */
public abstract class TemplateBasedFileWizard extends TemplateBasedWizard {

    private IWorkbench workbench;
    private IStructuredSelection selection;
    private WizardNewFileCreationPage firstPage;
    private String wizardType;

    public TemplateBasedFileWizard() {
    }
    
    public void init(IWorkbench workbench, IStructuredSelection currentSelection) {
        this.workbench = workbench;
        this.selection = currentSelection;
    }
    
    public WizardNewFileCreationPage getFirstPage() {
        return firstPage;
    }

    public String getWizardType() {
        return wizardType;
    }
    
    public void setWizardType(String wizardType) {
        this.wizardType = wizardType;
    }
    
    @Override
    public void addPages() {
        firstPage = new WizardNewFileCreationPage("file selection page", selection);
        firstPage.setAllowExistingResources(false);
        addPage(firstPage);
        super.addPages();
    }

    @Override
    protected List<IContentTemplate> getTemplates() {
        List<IContentTemplate> result = new ArrayList<IContentTemplate>();
        result.addAll(loadBuiltinTemplates(getWizardType()));
        result.addAll(loadTemplatesFromWorkspace(getWizardType()));
        return result;
    }
    
    @Override
    protected CreationContext createContext(IContentTemplate selectedTemplate, IContainer folder) {
        CreationContext context = super.createContext(selectedTemplate, folder);
        context.getVariables().put("wizardType", getWizardType());
        context.getVariables().put("newFileName", firstPage.getFileName());
        return context;
    }
    
    @Override
    public boolean performFinish() {
        boolean ok = super.performFinish();
        if (!ok)
            return false;
        
        // open the file for editing 
        IPath filePath = firstPage.getContainerFullPath().append(firstPage.getFileName());
        IFile newFile = ResourcesPlugin.getWorkspace().getRoot().getFile(filePath);
        if (!newFile.exists()) {
            MessageDialog.openError(getShell(), "Problem", "The wizard does not seem to have created the requested file:\n" + newFile.getFullPath().toString());
            return false;
        }
        
        try {
            IWorkbenchWindow dwindow = workbench.getActiveWorkbenchWindow();
            IWorkbenchPage page = dwindow.getActivePage();
            if (page != null)
                IDE.openEditor(page, newFile, true);
        } 
        catch (org.eclipse.ui.PartInitException e) {
            CommonPlugin.logError(e);
            return false;
        }
        return true;
    }

    @Override
    protected IWizardPage getFirstExtraPage() {
        return null;
    }

    @Override
    protected IContainer getFolder() {
        IPath path = firstPage.getContainerFullPath();
        IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
        return path.segmentCount()==1 ? root.getProject(path.toString()) : root.getFolder(path);
    }

}
