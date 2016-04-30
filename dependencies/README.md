There are 3 types of dependencies.

Dependencies prefixed by grit- are owned by the Grit project.  They may be forks or from-scratch
dependencies.  They live in their own repos but contain grit-compatible build scripts.

Dependencies suffixed by a version code are included not as submodules but directories within the
grit-engine repo.  This means there was no repo to clone but we did not want to create one as we had
not modified these dependencies and did not share this dependency among many projects.  We do
however include added build scripts within the dirs.

The remaining dependencies are clones of projects not owned by the Grit project.  To avoid forking
them to simply add build scripts, these scripts are added in the dependencies/ directory instead.
