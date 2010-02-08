/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.ned.core.ui.actions;

import java.io.ByteArrayInputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IResourceVisitor;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.dialogs.ProgressMonitorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;
import org.eclipse.ui.dialogs.ListDialog;
import org.omnetpp.ned.core.INedResources;
import org.omnetpp.ned.core.NEDResourcesPlugin;
import org.omnetpp.ned.core.refactoring.RefactoringTools;
import org.omnetpp.ned.model.ex.NedFileElementEx;

/**
 * Fixes package declarations, organizes imports, and reformats
 * source code in the selected directories.
 *
 * @author Andras
 */
//FIXME tell user: "Please save all files and close all editors" etc.
//FIXME tell user if some files has syntax error
public class CleanupNedFilesAction implements IWorkbenchWindowActionDelegate {
    // used internally
    interface INEDRefactoring {
        void process(NedFileElementEx nedFileElement);
    }

    INEDRefactoring pass1 = new INEDRefactoring() {
        public void process(NedFileElementEx nedFileElement) {
            RefactoringTools.cleanupTree(nedFileElement);
            RefactoringTools.fixupPackageDeclaration(nedFileElement);
        }
    };

    INEDRefactoring pass2 = new INEDRefactoring() {
        public void process(NedFileElementEx nedFileElement) {
            RefactoringTools.organizeImports(nedFileElement);
        }
    };

    public void init(IWorkbenchWindow window) {
    }

    public void dispose() {
    }

    public void selectionChanged(IAction action, ISelection selection) {
    }

    public void run(IAction action) {
        IWorkbenchWindow activeWorkbenchWindow = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		Shell shell = activeWorkbenchWindow == null ? null :activeWorkbenchWindow.getShell();

		ContainerSelectionDialog dialog = new ContainerSelectionDialog(shell, ResourcesPlugin.getWorkspace().getRoot(), false, "Clean up NED files in the following folder:");
        if (dialog.open() == ListDialog.OK) {
            IPath path = (IPath) dialog.getResult()[0];
            final IContainer container = (IContainer) ResourcesPlugin.getWorkspace().getRoot().findMember(path);

            try {
                IRunnableWithProgress op = new IRunnableWithProgress() {
                    public void run(IProgressMonitor monitor) throws InvocationTargetException, InterruptedException {
                        cleanupNedFilesIn(container, monitor);
                    }};
                // first param of run() is "fork" it set to false to run the process in UI thread to allow dialogs to come up
                new ProgressMonitorDialog(shell).run(false, true, op);
            }
            catch (InvocationTargetException e) {
                NEDResourcesPlugin.logError(e);
                ErrorDialog.openError(shell, "Error", "Error during cleaning up NED files", new Status(IMarker.SEVERITY_ERROR, NEDResourcesPlugin.PLUGIN_ID, e.getMessage(), e));
            } catch (InterruptedException e) {
                // nothing to do
            }
        }
    }

    protected void cleanupNedFilesIn(IContainer container, final IProgressMonitor monitor) {
        try {

        	NEDResourcesPlugin.getNEDResources().setRefactoringInProgress(true);
            NEDResourcesPlugin.getNEDResources().fireBeginChangeEvent();

            // we need to fix package declarations and imports in two separate passes
            container.accept(createVisitor(pass1, monitor)); // fix all package declarations
            container.accept(createVisitor(pass2, monitor)); // organize imports

            monitor.done();
        }
        catch (CoreException e) {
            NEDResourcesPlugin.logError(e);
            IWorkbenchWindow activeWorkbenchWindow = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
    		Shell shell = activeWorkbenchWindow == null ? null :activeWorkbenchWindow.getShell();
            ErrorDialog.openError(shell, "Error", "An error occurred during cleaning up NED files. Not all of the selected files have been processed.", e.getStatus());
        }
        finally {
            NEDResourcesPlugin.getNEDResources().fireEndChangeEvent();
            NEDResourcesPlugin.getNEDResources().setRefactoringInProgress(false);
        }
    }

    protected IResourceVisitor createVisitor(final INEDRefactoring refactoring, final IProgressMonitor monitor) {
        return new IResourceVisitor() {
            public boolean visit(IResource resource) throws CoreException {
                if (NEDResourcesPlugin.getNEDResources().isNedFile(resource)) {
                    monitor.subTask(resource.getFullPath().toString());

                    processNedFile((IFile)resource, refactoring);

                    // we are running in the UI thread. process to events to make the UI responsive
                    Display.getCurrent().readAndDispatch();
                    monitor.worked(1);
                }
                if (monitor.isCanceled())
                    return false;
                return true;
            }
        };
    }

    protected void processNedFile(IFile file, INEDRefactoring refactoring) throws CoreException {
        INedResources res = NEDResourcesPlugin.getNEDResources();
        NedFileElementEx nedFileElement = res.getNedFileElement(file);

        if (!nedFileElement.hasSyntaxError()) {
            String originalSource = nedFileElement.getNEDSource();

        	// do the actual work
            refactoring.process(nedFileElement);

        	// save the file if changed
        	String source = nedFileElement.getNEDSource();
        	if (!source.equals(originalSource))
        	    file.setContents(new ByteArrayInputStream(source.getBytes()), IFile.FORCE, null);
        }
    }

}
