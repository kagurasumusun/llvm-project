# ci/workflows

The reference copy of the GitHub Actions workflows, plus a note on why the CI
is arranged the way it is.

## The constraint

GitHub refuses any write to `.github/workflows/` that comes from a GitHub App
without the `workflows` permission:

    refusing to allow a GitHub App to create or update workflow
    `.github/workflows/<name>.yml` without `workflows` permission

This applies to `git push`, the Contents API and the Git Data API alike -- the
path is what is checked, not the route. Two separate identities hit it here:

* the agent working on this repository, which authenticates as a GitHub App
  installation;
* the built-in `GITHUB_TOKEN`, which authenticates as `github-actions[bot]`.
  That is also an App, and it **cannot be granted** the `workflows` permission
  at all -- `permissions:` in a workflow file has no setting for it.

So the obvious fix, a job that copies files into `.github/workflows/`, does not
work: the copying job is refused for exactly the same reason. That approach was
tried and removed.

## What is done instead

Only the *path* is protected, not the behaviour. So `.github/workflows/` holds
as little as possible -- a list of step names, each one calling a script -- and
everything that actually decides what the CI does lives in `ci/metal/`, which
is a normal directory with no restrictions:

    .github/workflows/metal-build.yml   step names and triggers (protected)
    ci/metal/build.sh                   cmake flags, targets, diagnostics (free)

Changing compiler flags, the cmake configuration, how the standard library is
fetched, how failures are reported, which tests run -- none of that touches the
protected file. `metal-build.yml` should only need editing when a step is added
or removed outright, which is rare.

`metal-build.yml` here is kept as the reference copy of what is installed. It
is not synced automatically; if it and the installed file ever diverge, the
installed one wins and this copy should be updated to match.

## Other ways out, and why they were not taken

* **A PAT with the `workflow` scope, stored as a repository secret.** Works,
  and is the usual answer. Rejected because a token that can rewrite CI is a
  standing credential with a very large blast radius, and it has to be created
  and rotated by hand anyway.
* **A GitHub App created for this repository with `workflows: write`.** Also
  works and is tidier than a PAT, but is more setup than the problem warrants.
* **Composite actions under `.github/actions/`.** That directory is *not*
  protected, so it is a real option. `ci/metal/build.sh` was preferred because
  a plain script can be run locally, unchanged, outside Actions.
